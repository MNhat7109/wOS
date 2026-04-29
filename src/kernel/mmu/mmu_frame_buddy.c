#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_buddy.h>
#include <kernel/mmu.h>
#include <bitmap.h>
#include <kernel/debug.h>

#define MAX_ORDER 10

#define MMU_BUDDY_ZONE_DMA 0
#define MMU_BUDDY_ZONE_NORMAL 1
#define MMU_BUDDY_ZONE_HIMEM 2

static struct
{
    u64 total_pages;
    mmu_frame_buddy_t* frame_data;
    mmu_frame_buddy_t* free_list[MAX_ORDER+1];
} mmu_frame_buddy_data;

void mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, u8* start_addr, u64 mem_size);
uptr mmu_frame_buddy_alloc(mmu_frame_allocator_t* m_alloc, u64 size);
void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr);

u32 mmu_frame_buddy_traverse_order(u32 order);
void mmu_frame_buddy_register_chunk(u32 pfn, u32 order);
void mmu_frame_buddy_split_chunks(u32 start_pfn, u32 len);
void mmu_frame_buddy_reserve(u32 pfn);
void mmu_frame_buddy_reserve_range(u32 pfn, u32 count);
void mmu_frame_buddy_merge(mmu_frame_buddy_t* chunk);
void mmu_frame_buddy_split(mmu_frame_buddy_t* chunk, u32 desired_order);

void mmu_frame_buddy_pop_from_free_list(u32 order);
void mmu_frame_buddy_push_to_free_list(u32 order, mmu_frame_buddy_t* frame);
void mmu_frame_buddy_erase_from_free_list(u32 order, mmu_frame_buddy_t* frame);
u32 mmu_frame_ceillog2(u64 x);
u32 mmu_frame_floorlog2(u64 x);

mmu_frame_allocator_ops_t alloc_buddy = 
{
    .alloc = &mmu_frame_buddy_alloc,
    .free = &mmu_frame_buddy_free,
    .init = &mmu_frame_buddy_init
};

void mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, u8* start_addr, u64 mem_size)
{
	if (!mmu_is_aligned((uptr)start_addr, PAGE_SIZE)) return; 

    
	mmu_frame_buddy_data.frame_data = (mmu_frame_buddy_t*)start_addr;
    mmu_frame_buddy_data.total_pages = (mem_size) / (PAGE_SIZE);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Buddy start addr: 0x%x\n", start_addr);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Total pages: %llu\n", mmu_frame_buddy_data.total_pages);

    usize meta_page_count = mmu_byte_to_4k_pages(sizeof(mmu_frame_buddy_t)*mmu_frame_buddy_data.total_pages);
    bitmap_t* bmp = m_alloc->mem_state;
    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap addr: 0x%x\n", bmp->buffer);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Frame addr: 0x%x\n", mmu_frame_buddy_data.frame_data);

    // Reserve spaces for the metadata
    uptr buddy_addr_phys = mmu_vtop((vaddr_t)start_addr);
    u32 buddy_meta_pfn = buddy_addr_phys >> 12;

    kdebugf(DEBUG_INFO, MODULE_MMU, "Reserving buddy metadata...\n");
    mmu_frame_buddy_reserve_range(buddy_meta_pfn, meta_page_count);
    
    kdebugf(DEBUG_INFO, MODULE_MMU, "Consolidating memory...\n");
    // Consolidate the current memory state from address 0x0
    // This needs to be done, in order to get a good free list of buddy chunks in various orders from 0 to 10.
    u32 current_pfn = 0, buddy_chunks = 0, round=0;
    while (current_pfn < mmu_frame_buddy_data.total_pages)
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Round %u...\n", round);
        kdebugf(DEBUG_INFO, MODULE_MMU, "Current PFN at index %u: %u\n", current_pfn, bitmap_get(bmp, current_pfn));
        if (bitmap_get(bmp, current_pfn) != 0)
        {
            mmu_frame_buddy_reserve(current_pfn);
            current_pfn++;
            kdebugf(DEBUG_INFO, MODULE_MMU, "Done reserving.\n", round);
            goto endloop;
        }
        
        u32 start_free_pfn = current_pfn;
        kdebugf(DEBUG_INFO, MODULE_MMU, "Acquiring free pages to split to order 10 blocks....\n", round);
        while (current_pfn < mmu_frame_buddy_data.total_pages
            ) 
            {
                if (bitmap_get(bmp, current_pfn)!=0) break;
                current_pfn++;
            }
            
            u32 free_len = current_pfn-start_free_pfn;
            
            kdebugf(DEBUG_INFO, MODULE_MMU, "Done acquiring.\n");
            mmu_frame_buddy_split_chunks(start_free_pfn, free_len);
endloop:
        round++;
    }
    
    // Summarize total buddy chunks
    kdebugf(DEBUG_INFO, MODULE_MMU, "Done getting info for buddy metadata. Summary:\n");
    for (u32 i=0;i<=MAX_ORDER;i++)
    {
        u32 order_count = mmu_frame_buddy_traverse_order(i);
        kdebugf(DEBUG_INFO, MODULE_MMU, "\tblock size=%u pages, count=%u\n", (1<<i), order_count);
        buddy_chunks+=order_count;
    }
    kdebugf(DEBUG_INFO, MODULE_MMU, "Total chunk count: %u\n", buddy_chunks);
}
    
uptr mmu_frame_buddy_alloc(mmu_frame_allocator_t* m_alloc, u64 page_count)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Allocating %llu pages...\n", page_count);

    // Compute the buddy order of the pages to allocate
    // We can use log2 to do this.
    // Here, if the page count is not a power of 2, we will have to round the order up.
    // For example: 
    //      Allocate 64 pages, get one block of order 6 (order 6 => 2^6 = 64-page block, nicely fit)
    //      Allocate 65 pages, get one block of order 7 (order 7 => 2^7 = 128-page block > 65 pages)
    u32 desired_order = mmu_frame_ceillog2(page_count);
    u32 order_found = desired_order;
    u32 pfn_offset = 0;
    mmu_frame_buddy_t* head_at_order=NULL;

    // Find available chunk of (desired_order). If unavailable, bump the order up and repeat.
    while (order_found <= MAX_ORDER)
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Trying to allocate one %llu-page block...\n", (u64)(1<<order_found));
        head_at_order = mmu_frame_buddy_data.free_list[order_found];
        if (head_at_order)
        {
            pfn_offset = head_at_order-mmu_frame_buddy_data.frame_data;
            break;
        }
        
        kdebugf(DEBUG_INFO, MODULE_MMU, "%llu-page block not found. Double it and wait until the next loop.\n", (u64)(1<<order_found));
        order_found++;
    }
    
    // If every memory order is exhausted, then no allocation.
    if (!head_at_order) return 0;
    
    // If we have to settle on a larger block than we need, split the block out.
    // This function ensures that the current available address is at the top, 
    // so that we can optimize the "occupied" marking stage, which is here below.
    
    if (order_found > desired_order)
    mmu_frame_buddy_split(head_at_order, desired_order);

    head_at_order->frame_attr.is_free = 0; // Used

    // Remove the item out of the free list, as it is "occupied"
    mmu_frame_buddy_erase_from_free_list(desired_order, head_at_order);

    return pfn_offset<<12;
}

void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Freeing block at 0x%x...\n", frame_addr);
    // Obtain the PFN for our address
    usize frame_no = frame_addr >> 12;

    mmu_frame_buddy_t* frame_ptr = &mmu_frame_buddy_data.frame_data[frame_no];

    // We will not free reserved or already freed block
    if (frame_ptr->frame_attr.reserved) return;
    if (frame_ptr->frame_attr.is_free) return;

    frame_ptr->frame_attr.is_free = 1;
    mmu_frame_buddy_merge(frame_ptr);
}

u32 mmu_frame_ceillog2(u64 x)
{
    if (x <= 1) return 0;

    x--;
    u32 shift = 0;

    while (x >>= 1)
        shift++;

    return shift + 1;
}

u32 mmu_frame_floorlog2(u64 x)
{
    if (x <= 1) return 0;

    u32 shift = 0;

    while (x >>= 1)
        shift++;

    return shift-1;
}

void mmu_frame_buddy_split(mmu_frame_buddy_t* chunk, u32 desired_order)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    // First, we pop the larger chunk
    mmu_frame_buddy_erase_from_free_list(chunk->frame_attr.order, chunk);

    // Then, on a loop we constantly do:
    //      1. Decrease order (which means cut that chunk in half)
    //      2. So now we have two halves. We might want to keep one half, and register the other as a free block at its order
    // We loop until the chunk's order is equal to what we need

    u32 pfn = (u32)(chunk - mmu_frame_buddy_data.frame_data);
    while (chunk->frame_attr.order > desired_order)
    {
        chunk->frame_attr.order--;
        // Here, to access the aforementioned "other half", we can use bitwise XOR.
        // At a specific order, to access a page frame's peer, do: (pfn ^ (1<<order))

        mmu_frame_buddy_register_chunk((pfn ^ (1<<chunk->frame_attr.order)), chunk->frame_attr.order);
    }
}

void mmu_frame_buddy_merge(mmu_frame_buddy_t* chunk)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    u32 chunk_pfn = (u32)(chunk-mmu_frame_buddy_data.frame_data);

    // First, on a loop we do:
    //      - Compute the other half.
    //      - Check if that half is either NOT reserved, is free, or its order being the same as the current half's. 
    // If so, it is eligible for merging.
    //      - Merge by removing the other half from the free list, and bumping up the order of the current half.

    while (chunk->frame_attr.order < MAX_ORDER)
    {
        u32 peer_chunk_pfn = chunk_pfn ^ (1<<chunk->frame_attr.order);
        mmu_frame_buddy_t* peer_chunk = &mmu_frame_buddy_data.frame_data[peer_chunk_pfn];

        if (peer_chunk->frame_attr.reserved) break;
        if (!peer_chunk->frame_attr.is_free) break;
        if (peer_chunk->frame_attr.order != chunk->frame_attr.order) break;

        mmu_frame_buddy_erase_from_free_list(chunk->frame_attr.order, peer_chunk);
        chunk->frame_attr.order++;
    }   

    // Finally, add the merged chunk back to the free list.
    mmu_frame_buddy_push_to_free_list(chunk->frame_attr.order, chunk);
}

void mmu_frame_buddy_split_chunks(u32 start_pfn, u32 len)
{
    // TODO: Also document this code
    kdebugf(DEBUG_INFO, MODULE_MMU, "Splitting from PFN %u, len=%u.\n", start_pfn, len);
    u32 current_pfn = start_pfn;
    while (len)
    {
        u32 nearest_smaller_order = mmu_frame_floorlog2(len);
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

void mmu_frame_buddy_reserve(u32 pfn)
{
    mmu_frame_buddy_data.frame_data[pfn].frame_attr.reserved = 1;
}

void mmu_frame_buddy_reserve_range(u32 pfn, u32 count)
{
    for (u32 i=0;i<count;i++)
        mmu_frame_buddy_reserve(pfn+i);
}

void mmu_frame_buddy_register_chunk(u32 pfn, u32 order)
{
    if (pfn >= mmu_frame_buddy_data.total_pages) return;

    mmu_frame_buddy_data.frame_data[pfn].frame_attr = (mmu_frame_buddy_attr_t){
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

u32 mmu_frame_buddy_traverse_order(u32 order)
{
    u32 count=0;
    if (order > MAX_ORDER) return count;
    mmu_frame_buddy_t* head = mmu_frame_buddy_data.free_list[order];
    while (head)
    {
        count++;
        head = head->next;
    }
    return count;
}

void mmu_frame_buddy_pop_from_free_list(u32 order)
{
    if (order > MAX_ORDER) return;

    mmu_frame_buddy_t* head = mmu_frame_buddy_data.free_list[order];
    if (!head) return;

    mmu_frame_buddy_data.free_list[order] = head->next;
    head->next = NULL;
}

void mmu_frame_buddy_push_to_free_list(u32 order, mmu_frame_buddy_t* frame)
{
    if (!frame) return;
    if (order > MAX_ORDER) return;

    mmu_frame_buddy_t* head = mmu_frame_buddy_data.free_list[order];

    frame->next = head;
    mmu_frame_buddy_data.free_list[order] = frame;
}

void mmu_frame_buddy_erase_from_free_list(u32 order, mmu_frame_buddy_t* frame)
{
    if (!frame) return;
    if (order > MAX_ORDER) return;

    mmu_frame_buddy_t** pp = &mmu_frame_buddy_data.free_list[order];

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
