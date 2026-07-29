#include <stdint.h>
#include <stdbool.h>
#include <kernel/debug.h>
#include <kernel/mmu.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_buddy.h>
#include <bitmap.h>

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

void mmu_frame_buddy_merge(mmu_frame_buddy_t* chunk);
void mmu_frame_buddy_split(mmu_frame_buddy_t* chunk, u32 desired_order);

void mmu_frame_buddy_migrate_from_bitmap(uptr start_addr, u64 mem_size);

const bitmap_t* mmu_frame_bmp_get_metadata();
u32 mmu_frame_buddy_traverse_order(u32 order);
void mmu_frame_buddy_register_chunk(u32 pfn, u32 order);
void mmu_frame_buddy_split_chunks(u32 start_pfn, u32 len);

int mmu_frame_buddy_mark_reserved(u32 pfn);
int mmu_frame_buddy_mark_reserved_n(u32 pfn, u32 count);
void mmu_frame_buddy_mark_free(u32 pfn);
void mmu_frame_buddy_mark_used(u32 pfn);
u8 mmu_frame_buddy_get_order(u32 pfn);

void mmu_frame_buddy_pop_from_free_list(u32 order);
void mmu_frame_buddy_push_to_free_list(u32 order, mmu_frame_buddy_t* frame);
void mmu_frame_buddy_erase_from_free_list(u32 order, mmu_frame_buddy_t* frame);
bool mmu_frame_buddy_check_reserved(u32 pfn);
bool mmu_frame_buddy_check_free(u32 pfn);

u32 mmu_ceillog2(u64 x);
u32 mmu_floorlog2(u64 x);

void mmu_frame_buddy_migrate_from_bitmap(uptr start_addr, u64 mem_size)
{
    mmu_frame_buddy_data.frame_data = (mmu_frame_buddy_t*)start_addr;
    mmu_frame_buddy_data.total_pages = mmu_byte_to_4k_pages(mem_size);
    
    bitmap_t* bmp = mmu_frame_bmp_get_metadata();

    // To prepare for the migration, invalidate the bitmap buffer from the PMM.
    // We will not unmap the buffer yet, because we still need to do the migration of used/free pages 
    // from the old bitmap allocator to the newer buddy one.
    mmu_frame_release_pages(mmu_vtop((vaddr_t)bmp->buffer), mmu_byte_to_4k_pages(bmp->size));
    
    kdebugf(DEBUG_INFO, MODULE_MMU, "Frame addr: 0x%x\n", mmu_frame_buddy_data.frame_data);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Total pages: %llu\n", mmu_frame_buddy_data.total_pages);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap addr: 0x%x\n", bmp->buffer);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Consolidating memory...\n");
    // Consolidate the current memory state from address 0x0
    // This needs to be done, in order to get a good free list of buddy chunks in various orders from 0 to 10.
    u32 current_pfn = 0, buddy_chunks = 0, round=0;
    while (current_pfn < mmu_frame_buddy_data.total_pages)
    {
        if (bitmap_get(bmp, current_pfn) != 0)
        {
            mmu_frame_buddy_mark_reserved(current_pfn);
            current_pfn++;
            goto endloop;
        }
        
        u32 start_free_pfn = current_pfn;
        while (current_pfn < mmu_frame_buddy_data.total_pages
            ) 
            {
                if (bitmap_get(bmp, current_pfn)!=0) break;
                current_pfn++;
            }
            
            u32 free_len = current_pfn-start_free_pfn;
            
            mmu_frame_buddy_split_chunks(start_free_pfn, free_len);
endloop:
        round++;
    }

    // Unmap the bitmap buffer
    //mmu_vmem_free((vaddr_t)bmp->buffer);
    
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

///////////////////////////////////////////////////////////////////////////

void mmu_frame_buddy_split(mmu_frame_buddy_t* chunk, u32 desired_order)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    u32 pfn = (u32)(chunk - mmu_frame_buddy_data.frame_data);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Splitting chunk (header offset=0x%p, pfn=%u) to order %u...\n", chunk, pfn, desired_order);

    // First, we pop the larger chunk
    mmu_frame_buddy_erase_from_free_list(chunk->frame_attr.order, chunk);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Erased chunk out of free list.\n");

    // Then, on a loop we constantly do:
    //      1. Decrease order (which means cut that chunk in half)
    //      2. So now we have two halves. We might want to keep one half, and register the other as a free block at its order
    // We loop until the chunk's order is equal to what we need

    while (chunk->frame_attr.order > desired_order)
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Kage Bunshin no Jutsu!\n");
        chunk->frame_attr.order--;
        // Here, to access the aforementioned "other half", we can use bitwise XOR.
        // At a specific order, to access a page frame's peer, do: (pfn ^ (1<<order))
        
        // kdebugf(DEBUG_INFO, MODULE_MMU, "Adding block at PFN=%u, order=%u to free list...\n", 
            // (pfn ^ (1<<chunk->frame_attr.order)), chunk->frame_attr.order
        // );
        mmu_frame_buddy_register_chunk((pfn ^ (1<<chunk->frame_attr.order)), chunk->frame_attr.order);
    }
    kdebugf(DEBUG_INFO, MODULE_MMU, "Done splitting chunk.\n");
}

void mmu_frame_buddy_merge(mmu_frame_buddy_t* chunk)
{
    if (!chunk) return;
    if (!chunk->frame_attr.is_free) return;

    kdebugf(DEBUG_INFO, MODULE_MMU, "Merging chunk at 0x%x...\n", chunk);

    u32 chunk_pfn = (u32)(chunk-mmu_frame_buddy_data.frame_data);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Chunk PFN=%u\n", chunk_pfn);

    // First, on a loop we do:
    //      - Compute the other half's PFN.
    //      - Check if that half is either NOT reserved, is free, or its order being the same as the current half's. 
    // If so, it is eligible for merging.
    //      - Merge by removing the other half from the free list, and bumping up the order of the current half.

    while (chunk->frame_attr.order < MAX_ORDER)
    {
        // kdebugf(DEBUG_INFO, MODULE_MMU, "Current order=%u...\n", chunk->frame_attr.order);
        u32 peer_chunk_pfn = chunk_pfn ^ (1<<chunk->frame_attr.order);
        mmu_frame_buddy_t* peer_chunk = &mmu_frame_buddy_data.frame_data[peer_chunk_pfn];
        // kdebugf(DEBUG_INFO, MODULE_MMU, "Examining chunk PFN=%u...\n", peer_chunk_pfn);

        if (peer_chunk->frame_attr.reserved) break;
        if (!peer_chunk->frame_attr.is_free) break;
        if (peer_chunk->frame_attr.order != chunk->frame_attr.order) break;

        // kdebugf(DEBUG_INFO, MODULE_MMU, "Eligible for rejoining. Erasing from free list...\n");
        mmu_frame_buddy_erase_from_free_list(chunk->frame_attr.order, peer_chunk);
        kdebugf(DEBUG_INFO, MODULE_MMU, "Fuu- sion... HA!\n");
        chunk->frame_attr.order++;
    }   

    kdebugf(DEBUG_INFO, MODULE_MMU, "Chunk at PFN=%u is now at order=%u, pushing to free list...\n", chunk_pfn, chunk->frame_attr.order);
    // Finally, add the merged chunk back to the free list.
    mmu_frame_buddy_push_to_free_list(chunk->frame_attr.order, chunk);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Done merging chunk.\n");
}


u32 mmu_frame_buddy_find_avl_block(u32 order)
{
    u32 desired_order = order;
    u32 order_found = desired_order;
    u32 pfn_offset = 0;
    mmu_frame_buddy_t* head_at_order=NULL;

    // Find available chunk of (desired_order). If unavailable, bump the order up and repeat.
    while (order_found <= MAX_ORDER)
    {
        // kdebugf(DEBUG_INFO, MODULE_MMU, "Trying to allocate one %llu-page block...\n", (u64)(1<<order_found));
        head_at_order = mmu_frame_buddy_data.free_list[order_found];
        if (head_at_order)
        {
            // kdebugf(DEBUG_INFO, MODULE_MMU, "%llu-page block found.\n", (u64)(1<<order_found));
            pfn_offset = head_at_order-mmu_frame_buddy_data.frame_data;
            break;
        }
        
        // kdebugf(DEBUG_INFO, MODULE_MMU, "%llu-page block not found. Double it and wait until the next loop.\n", (u64)(1<<order_found));
        order_found++;
    }
    
    // If every memory order is exhausted, then no allocation.
    if (!head_at_order) return 0;
    
    // If we have to settle on a larger block than we need, split the block out.

    if (order_found > desired_order)
    mmu_frame_buddy_split(head_at_order, desired_order);
    else
    mmu_frame_buddy_erase_from_free_list(desired_order, head_at_order);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Free block found at PFN=%u, capacity=%llu pages\n", pfn_offset, (u64)(1<<head_at_order->frame_attr.order));
    return pfn_offset;
}

void mmu_frame_buddy_coalesce_block(u32 pfn)
{
    mmu_frame_buddy_t* frame_ptr = &mmu_frame_buddy_data.frame_data[pfn];

    // We will not free reserved or already freed block
    if (mmu_frame_buddy_check_reserved(pfn)) 
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Sorry, you discovered our secret club at this block. "
            "We may let you off with a warning, but you won't be that lucky the second time...\n"
        );
        return;
    }
    if (mmu_frame_buddy_check_free(pfn)) 
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "This has already been freed. Here's your dementia pill...\n"
        );
        return;
    }

    mmu_frame_buddy_mark_free(pfn);
    mmu_frame_buddy_merge(frame_ptr);
}

void mmu_frame_buddy_register_chunk(u32 pfn, u32 order)
{
    if (pfn >= mmu_frame_buddy_data.total_pages) return;
    mmu_frame_buddy_data.frame_data[pfn].frame_attr = (mmu_frame_buddy_attr_t){
        .is_free = 1,
        .reserved = 0,
        .order = order,
        .zone = MMU_BUDDY_ZONE_NORMAL,
        .is_head = 1
    };
    mmu_frame_buddy_data.frame_data[pfn].next = NULL;
    
    if (!mmu_frame_buddy_data.free_list[order])
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Making new linked list at order %u...\n", order);
        mmu_frame_buddy_data.free_list[order] = &mmu_frame_buddy_data.frame_data[pfn];
        return;
    }
    
    mmu_frame_buddy_push_to_free_list(order, &mmu_frame_buddy_data.frame_data[pfn]);
}


void mmu_frame_buddy_split_chunks(u32 start_pfn, u32 len)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Splitting from PFN %u, len=%u.\n", start_pfn, len);

    // We split the previous giant blob of free space in RAM 
    // to small manageable chunks of 1024-page and down

    // To do so:
    // - Find the maximum log2 that has its corresponding (1<<log2) to be smaller than the length of chunk.
    // This will be the (nearest_smaller_order), and (1<<nearest_smaller_order) will be the chunk's smaller power-of-two.
    // - Check if the PFN is aligned with the power-of-two. If not, decrease the order and repeat.
    // - Once we found our order, register the block.
    // - Increase the PFN and decrease the length of chunk by (1<<nearest_smaller_order)
    // - Repeat until there's nothing left.
    
    u32 current_pfn = start_pfn;
    while (len)
    {
        u32 nearest_smaller_order = mmu_floorlog2(len);
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


//////////////////////////////////////////////////////////////////////////

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

bool mmu_frame_buddy_check_reserved(u32 pfn)
{
    return mmu_frame_buddy_data.frame_data[pfn].frame_attr.reserved;
}

bool mmu_frame_buddy_check_free(u32 pfn)
{
    return mmu_frame_buddy_data.frame_data[pfn].frame_attr.is_free;
}

bool mmu_frame_buddy_check_head(u32 pfn)
{
    return mmu_frame_buddy_data.frame_data[pfn].frame_attr.is_head;
}

int mmu_frame_buddy_mark_reserved(u32 pfn)
{
    if (!mmu_frame_buddy_check_free(pfn)) return -1;
    if (!mmu_frame_buddy_check_reserved(pfn)) return -1;
    mmu_frame_buddy_data.frame_data[pfn].frame_attr.reserved = 1;
    mmu_inc_zone_size(MMU_ZONE_OS_RESERVED, 4096);
    mmu_dec_zone_size(MMU_ZONE_FREE, 4096);
    return 0;
}

int mmu_frame_buddy_mark_reserved_n(u32 pfn, u32 count)
{
    for (u32 i=0;i<count;i++)
    {
        int status = mmu_frame_buddy_mark_reserved(pfn+i);
        if (status < 0) return -1;
    }
    return 0;
}

void mmu_frame_buddy_mark_free(u32 pfn)
{
    mmu_frame_buddy_data.frame_data[pfn].frame_attr.is_free = 1;
}

void mmu_frame_buddy_mark_used(u32 pfn)
{
    mmu_frame_buddy_data.frame_data[pfn].frame_attr.is_free = 0;
}

u8 mmu_frame_buddy_get_order(u32 pfn)
{
    return mmu_frame_buddy_data.frame_data[pfn].frame_attr.order;
}
