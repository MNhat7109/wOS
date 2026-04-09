#include <kernel/mmu_frame.h>
#include <kernel/mmu.h>
#include <bitmap.h>
#include <kernel/debug.h>

#define MAX_ORDER 10

#define MMU_BUDDY_ZONE_DMA 0
#define MMU_BUDDY_ZONE_NORMAL 1
#define MMU_BUDDY_ZONE_HIMEM 2

typedef struct mmu_frame_t mmu_frame_t;

typedef struct mmu_frame_attr_t
{
    u8 reserved : 1;
    u8 is_free : 1;
    u8 zone : 2;
    u8 order: 4;
} __attribute__((packed)) mmu_frame_attr_t;

typedef struct mmu_frame_t
{
    mmu_frame_attr_t frame_attr;
    mmu_frame_t* next;
} mmu_frame_t;

static struct
{
    u64 total_pages;
    mmu_frame_t* frame_data;
    mmu_frame_t* free_list[MAX_ORDER+1];
} mmu_frame_buddy_data;

void mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, u8* start_addr, u64 mem_size);
uptr mmu_frame_buddy_alloc(mmu_frame_allocator_t* m_alloc, u64 size);
void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr);

void mmu_frame_buddy_register_chunk(u32 pfn, u32 order);
void mmu_frame_buddy_split_to_4m(u32 start_pfn, u32 len);
void mmu_frame_buddy_merge(mmu_frame_t* chunk);
void mmu_frame_buddy_split(mmu_frame_t* chunk, u32 desired_order);

void mmu_frame_buddy_pop_from_free_list(u32 order);
void mmu_frame_buddy_push_to_free_list(u32 order, mmu_frame_t* frame);
void mmu_frame_buddy_erase_from_free_list(u32 order, mmu_frame_t* frame);
u32 mmu_frame_plog2(u64 x);

mmu_frame_allocator_ops_t alloc_buddy = 
{
    .alloc = &mmu_frame_buddy_alloc,
    .free = &mmu_frame_buddy_free,
    .init = &mmu_frame_buddy_init
};

void mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, u8* start_addr, u64 mem_size)
{
	if (!mmu_is_aligned((uptr)start_addr, PAGE_SIZE)) return; 

    kdebugf(DEBUG_INFO, MODULE_MMU, "Buddy start addr: 0x%x\n", start_addr);
    
	mmu_frame_buddy_data.frame_data = (mmu_frame_t*)start_addr;
    mmu_frame_buddy_data.total_pages = (mem_size) / (PAGE_SIZE);

    usize meta_page_count = mmu_byte_to_4k_pages(sizeof(mmu_frame_t)*mmu_frame_buddy_data.total_pages);
    bitmap_t* bmp = m_alloc->mem_state;

    kdebugf(DEBUG_INFO, MODULE_MMU, "Bmp addr: 0x%x\n", bmp->buffer);

    // Reserve spaces for the metadata
    u32 buddy_meta_pfn = (uptr)start_addr >> 12;
    mmu_frame_buddy_reserve_range(buddy_meta_pfn, meta_page_count);

    // Consolidate the current memory state from address 0x0
    // This needs to be done, in order to get a good free list of buddy chunks, in various orders from 0 to 10.
    u32 current_pfn = 0;
    while (current_pfn < mmu_frame_buddy_data.total_pages)
    {
        if (bitmap_get(bmp, current_pfn) != 0)
        {
            //mmu_frame_buddy_data.frame_data[current_pfn].frame_attr.reserved = 1;
            mmu_reserve_frame(current_pfn);
            current_pfn++;
            continue;
        }

        u32 start_free_pfn = current_pfn;
        while (current_pfn < mmu_frame_buddy_data.total_pages && 
        bitmap_get(bmp, current_pfn)==0) current_pfn++;

        u32 free_len = current_pfn-start_free_pfn;
        
        mmu_frame_buddy_split_to_4m(start_free_pfn, free_len);
    }
}

uptr mmu_frame_buddy_alloc(mmu_frame_allocator_t* m_alloc, u64 page_count)
{
    u32 desired_order = mmu_frame_plog2(page_count)+1;
    u32 order_found = desired_order;
    u32 pfn_offset = 0;
    mmu_frame_t* head_at_order;

    while (order_found <= MAX_ORDER)
    {
        head_at_order = mmu_frame_buddy_data.free_list[order_found];
        if (head_at_order)
        {
            pfn_offset = head_at_order-mmu_frame_buddy_data.frame_data;
            break;
        }
        
        order_found++;
    }
    
    if (!head_at_order) return 0;
    
    if (order_found > desired_order)
    mmu_frame_buddy_split(head_at_order, desired_order);

    head_at_order->frame_attr.is_free = 0;
    mmu_frame_buddy_pop_from_free_list(desired_order);

    return pfn_offset<<12;
}

void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr)
{
    usize frame_no = frame_addr >> 12;

    mmu_frame_t* frame_ptr = &mmu_frame_buddy_data.frame_data[frame_no];

    if (frame_ptr->frame_attr.reserved) return;
    if (frame_ptr->frame_attr.is_free) return;

    frame_ptr->frame_attr.is_free = 1;
    mmu_frame_buddy_merge(frame_ptr);
}

u32 mmu_frame_plog2(u64 x)
{
    u32 shift=0;
    while (x != 1)
    {
        x>>=1;
        shift++;
    }
    return shift;
}

void mmu_frame_buddy_split(mmu_frame_t* chunk, u32 desired_order)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    u32 pfn = (u32)(chunk - mmu_frame_buddy_data.frame_data);
    while (chunk->frame_attr.order > desired_order)
    {
        chunk->frame_attr.order--;
        mmu_frame_buddy_register_chunk((pfn ^ (1<<chunk->frame_attr.order)), chunk->frame_attr.order);
    }
}

void mmu_frame_buddy_merge(mmu_frame_t* chunk)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    u32 chunk_pfn = (u32)(chunk-mmu_frame_buddy_data.frame_data);

    while (chunk->frame_attr.order < MAX_ORDER)
    {
        u32 peer_chunk_pfn = chunk_pfn ^ (1<<chunk->frame_attr.order);
        mmu_frame_t* peer_chunk = &mmu_frame_buddy_data.frame_data[peer_chunk_pfn];

        if (peer_chunk->frame_attr.reserved) break;
        if (!peer_chunk->frame_attr.is_free) break;
        if (peer_chunk->frame_attr.order != chunk->frame_attr.order) break;

        mmu_frame_buddy_erase_from_free_list(chunk->frame_attr.order, peer_chunk);
        chunk->frame_attr.order++;
    }   
    mmu_frame_buddy_push_to_free_list(chunk->frame_attr.order, chunk);
}

void mmu_frame_buddy_split_to_4m(u32 start_pfn, u32 len)
{
    u32 current_pfn = start_pfn;
    while (len)
    {
        u32 nearest_smaller_order = mmu_frame_plog2(len);
        u32 size = (1<<nearest_smaller_order);
        while (nearest_smaller_order && (current_pfn & (size-1))) 
        {
            nearest_smaller_order--;
            size >>=1;
        }

        mmu_frame_buddy_register_chunk(current_pfn, nearest_smaller_order);
        current_pfn += size;
        len -= size;
    }
}

void mmu_frame_buddy_register_chunk(u32 pfn, u32 order)
{
    if (pfn >= mmu_frame_buddy_data.total_pages) return;

    mmu_frame_buddy_data.frame_data[pfn].frame_attr = (mmu_frame_attr_t){
        .is_free = 1,
        .reserved = 0,
        .order = order,
        .zone = MMU_BUDDY_ZONE_NORMAL
    };
    mmu_frame_buddy_data.frame_data[pfn].next = NULL;
    
    if (!mmu_frame_buddy_data.free_list[order])
    {
        mmu_frame_buddy_data.free_list[order] = &mmu_frame_buddy_data.frame_data[pfn];
        return;
    }

    mmu_frame_buddy_push_to_free_list(order, &mmu_frame_buddy_data.frame_data[pfn]);
}

void mmu_frame_buddy_pop_from_free_list(u32 order)
{
    if (order > MAX_ORDER) return;

    mmu_frame_t* head = mmu_frame_buddy_data.free_list[order];
    if (!head) return;

    mmu_frame_buddy_data.free_list[order] = head->next;
    head->next = NULL;
}

void mmu_frame_buddy_push_to_free_list(u32 order, mmu_frame_t* frame)
{
    if (!frame) return;
    if (order > MAX_ORDER) return;

    mmu_frame_t* head = mmu_frame_buddy_data.free_list[order];

    frame->next = head;
    mmu_frame_buddy_data.free_list[order] = frame;
}

void mmu_frame_buddy_erase_from_free_list(u32 order, mmu_frame_t* frame)
{
    if (!frame) return;
    if (order > MAX_ORDER) return;

    mmu_frame_t** pp = &mmu_frame_buddy_data.free_list[order];

    while (pp)
    {
        if (*pp == frame)
        {
            *pp = frame->next;
            break;
        }

        pp = &(*pp)->next;
    }
}

const mmu_frame_allocator_ops_t* mmu_frame_buddy_load_ops()
{
	return &alloc_buddy;
}
