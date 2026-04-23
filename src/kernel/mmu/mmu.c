#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_bmp.h>
#include <kernel/mmu_frame_buddy.h>
#include <kernel/mmu_other.h>
#include <kernel/debug.h>

mmu_frame_allocator_t* mmu_frame_alloc;
static struct
{
    uptr mmu_start_addr;
} mmu_data;


const mmu_frame_allocator_ops_t* mmu_frame_buddy_load_ops();

int mmu_init(uptr start_addr, memory_info_t* mem_map)
{
    if (!mem_map) return -1;

    mmu_data.mmu_start_addr = start_addr;

    mmu_recompute_mem_size(mem_map);

    // Init frame allocator
    mmu_frame_load_allocator(&mmu_frame_alloc);
    mmu_frame_load_ops(mmu_frame_alloc, mmu_frame_bmp_load_ops);
    kdebugf(DEBUG_INFO, MODULE_MMU, "%x\n", mmu_frame_alloc);
    mmu_frame_alloc->ops->init(mmu_frame_alloc, (u8*)start_addr, mmu_get_total_size());
    return 0;
}

void mmu_init_stage2()
{
    u8* start_addr_of_buddy = (u8*)mmu_align_up(mmu_data.mmu_start_addr+mmu_frame_alloc->mem_state->size, PAGE_SIZE);
    paddr_t start_addr_of_buddy_phys = mmu_vtop((vaddr_t)start_addr_of_buddy);
    u64 total_size = mmu_get_total_size();
    usize total_page_count = mmu_byte_to_4k_pages(total_size);
    usize buddy_page_count = mmu_byte_to_4k_pages(total_page_count*sizeof(mmu_frame_buddy_t));

    ((mmu_frame_bmp_allocator_ops_t*)mmu_frame_alloc->ops)->lock_pages(start_addr_of_buddy_phys, buddy_page_count);
    mmu_mmapn(start_addr_of_buddy_phys, buddy_page_count, MMU_PG_ATTR_RW, 0);
    
    
    mmu_frame_load_ops(mmu_frame_alloc, mmu_frame_buddy_load_ops);
    mmu_frame_alloc->ops->init(mmu_frame_alloc, start_addr_of_buddy, mmu_get_total_size());
    return;
}