#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_bmp.h>
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

    // Make use of mem_map for basic memory size, and size of each category (used, free, etc.)
    u64 memory_end=0;
    for (u32 i=0;i<mem_map->entries_count;i++)
    {
        memory_region_t* region = &mem_map->regions[i];
        int desired_zone_type;
        if (memory_end < region->base)
        {
            kdebugf(DEBUG_INFO, MODULE_MMU, "Added hole size %llu\n", region->base-memory_end);
            desired_zone_type = MMU_ZONE_HOLE;
            mmu_inc_zone_size(desired_zone_type, region->base-memory_end);
        }

        switch (region->type)
        {
            case MEMORY_TYPE_FREE:
                desired_zone_type = MMU_ZONE_FREE;
                break;
            case MEMORY_TYPE_RESERVED:
            case MEMORY_TYPE_ACPI:
            case MEMORY_TYPE_ACPI_NVS:
                desired_zone_type = MMU_ZONE_HW_RESERVED;
                break;
            case MEMORY_TYPE_BAD:
                desired_zone_type = MMU_ZONE_BAD;
                break;
            default:
                desired_zone_type = MMU_ZONE_OTHER;
                break;
        }

        kdebugf(DEBUG_INFO, MODULE_MMU, "Region start=0x%llx, len=%llu\n", region->base, region->length);

        mmu_inc_zone_size(desired_zone_type, region->length);
        memory_end=region->base+region->length;
    }

    mmu_recompute_total_size();

    // Init frame allocator
    mmu_frame_load_allocator(&mmu_frame_alloc);
    mmu_frame_load_ops(mmu_frame_alloc, mmu_frame_bmp_load_ops);
    kdebugf(DEBUG_INFO, MODULE_MMU, "%x\n", mmu_frame_alloc);
    mmu_frame_alloc->ops->init(mmu_frame_alloc, (u8*)start_addr, mmu_get_total_size());
    return 0;
}

void mmu_init_stage2()
{
    mmu_frame_load_ops(mmu_frame_alloc, mmu_frame_buddy_load_ops);
    u8* start_addr_of_buddy = (u8*)mmu_align_up(mmu_data.mmu_start_addr+mmu_frame_alloc->mem_state->size, PAGE_SIZE);
    mmu_frame_alloc->ops->init(mmu_frame_alloc, start_addr_of_buddy, mmu_get_total_size());
}