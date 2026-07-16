#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_buddy.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu.h>
#include <kernel/mmu_vmem.h>
#include <kernel/debug.h>
#include <stdbool.h>

#define MAX_ORDER 10

int mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, uptr start_addr, u64 mem_size);
uptr mmu_frame_buddy_alloc(mmu_frame_allocator_t* m_alloc, u64 size);
void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr);
void mmu_frame_buddy_reserve_pages(mmu_frame_allocator_t* m_alloc, uptr frame_addr, u64 n);
void mmu_frame_buddy_release_pages(mmu_frame_allocator_t* m_alloc, uptr frame_addr, u64 n);

void mmu_frame_buddy_migrate_from_bitmap(uptr start_addr, u64 mem_size);

int mmu_frame_buddy_mark_reserved(u32 pfn);
int mmu_frame_buddy_mark_reserved_n(u32 pfn, u32 count);
void mmu_frame_buddy_mark_free(u32 pfn);
void mmu_frame_buddy_mark_used(u32 pfn);
bool mmu_frame_buddy_check_reserved(u32 pfn);
bool mmu_frame_buddy_check_free(u32 pfn);
bool mmu_frame_buddy_check_head(u32 pfn);
u8 mmu_frame_buddy_get_order(u32 pfn);

u32 mmu_frame_buddy_find_avl_block(u32 order);
void mmu_frame_buddy_coalesce_block(u32 pfn);

u32 mmu_ceillog2(u64 x);
u32 mmu_floorlog2(u64 x);

mmu_frame_plugins_t buddy_plugin = {
    .init = &mmu_frame_buddy_init,
    .ops = {
        .alloc = &mmu_frame_buddy_alloc,
        .free = &mmu_frame_buddy_free,
        .reserve_pages = &mmu_frame_buddy_reserve_pages,
        .lock_pages = &mmu_frame_buddy_reserve_pages,
        .release_pages = &mmu_frame_buddy_release_pages
    }
};

int mmu_frame_buddy_init(mmu_frame_allocator_t* m_alloc, uptr start_addr, u64 mem_size)
{
	if (!mmu_is_aligned((uptr)start_addr, PAGE_SIZE)) return -1; 
    
    m_alloc->meta_offset_vaddr = start_addr;
    m_alloc->meta_size = sizeof(mmu_frame_buddy_t)*mmu_byte_to_4k_pages(mem_size);

    // Reserve spaces for the metadata
    kdebugf(DEBUG_INFO, MODULE_MMU, "Reserving buddy metadata...\n");
    void* meta_addr = mmu_vmem_alloc((void*)start_addr, m_alloc->meta_size, MMU_VMA_ANON | MMU_VMA_R | MMU_VMA_W, NULL);
    
    kdebugf(DEBUG_INFO, MODULE_MMU, "Buddy start addr: 0x%x\n", (uptr)meta_addr);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Buddy size: %llu\n", m_alloc->meta_size);

    mmu_frame_buddy_migrate_from_bitmap((uptr)meta_addr, mem_size);
    return 0;
}

void mmu_frame_buddy_reserve_pages(mmu_frame_allocator_t* m_alloc, uptr frame_addr, u64 n)
{
    u32 pfn = frame_addr >> 12;
    mmu_frame_buddy_mark_reserved_n(pfn, n);
}

void mmu_frame_buddy_release_pages(mmu_frame_allocator_t* m_alloc, uptr frame_addr, u64 n)
{

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
    u32 desired_order = mmu_ceillog2(page_count);
    u32 order_found = desired_order;
    u32 pfn_offset = mmu_frame_buddy_find_avl_block(desired_order);
    mmu_frame_buddy_mark_used(pfn_offset);

    mmu_inc_zone_size(MMU_ZONE_USED, PAGE_SIZE*(1<<desired_order));
    mmu_dec_zone_size(MMU_ZONE_FREE, PAGE_SIZE*(1<<desired_order));
    return pfn_offset<<12;
}

void mmu_frame_buddy_free(mmu_frame_allocator_t* m_alloc, uptr frame_addr)
{
    if (!mmu_is_aligned(frame_addr, PAGE_SIZE))
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Physical address is not aligned to PAGE_SIZE=0x1000.\n");
        return;
    }
    
    usize frame_no = frame_addr >> 12;
    if (!mmu_frame_buddy_check_head(frame_no))
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Tried to free page at middle of block.\n");
        return;
    }

    kdebugf(DEBUG_INFO, MODULE_MMU, "Freeing block at 0x%x...\n", frame_addr);
    // Obtain the PFN for our address
    u8 order = mmu_frame_buddy_get_order(frame_no);

    mmu_frame_buddy_coalesce_block(frame_no);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Done freeing.\n");

    mmu_inc_zone_size(MMU_ZONE_FREE, PAGE_SIZE*(1<<order));
    mmu_dec_zone_size(MMU_ZONE_USED, PAGE_SIZE*(1<<order));
}

const mmu_frame_plugins_t* mmu_frame_buddy_get_plugin()
{
	return &buddy_plugin;
}
