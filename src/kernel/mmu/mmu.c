#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu_other.h>
#include <kernel/debug.h>

static struct
{
    uptr mmu_start_addr;
} mmu_data;

const mmu_frame_plugins_t* mmu_frame_bmp_get_plugin();
const mmu_frame_plugins_t* mmu_frame_buddy_get_plugin();


int mmu_init(uptr start_addr, memory_info_t* mem_map)
{
    if (!mem_map) return -1;

    mmu_data.mmu_start_addr = mmu_align_up(start_addr, PAGE_SIZE);

    mmu_recompute_mem_size(mem_map);
    mmu_vmem_init(); // TODO

    // Init frame allocator
    kdebugf(DEBUG_INFO, MODULE_MMU, "Start addr: 0x%x\n", mmu_data.mmu_start_addr);
    int status = mmu_frame_populate(mmu_frame_bmp_get_plugin(), mmu_data.mmu_start_addr, mmu_get_total_size()); 
    if (status <0) return status;

    return 0;
}

int mmu_init_stage2(uptr starting_range)
{
    // PMM carry-over
    int status = mmu_frame_populate(mmu_frame_buddy_get_plugin(), starting_range, mmu_get_total_size()); 
    if (status < 0) return -1;
    
    // Heap kickstart
    // TODO
    return 0;
}