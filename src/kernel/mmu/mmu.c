#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_other.h>
#include <kernel/debug.h>

void mmu_reserve_low_memory();

int mmu_init(uptr start_addr, memory_info_t* mem_map)
{
    if (!mem_map) return -1;

    // Make use of mem_map for basic memory size, and size of each category (used, free, etc.)
    u64 memory_end=0;
    for (u32 i=0;i<mem_map->entries_count;i++)
    {
        memory_region_t* region = &mem_map->regions[i];
        int desired_zone_type;
        if (memory_end < region->base)
        {
            kdebugf(DEBUG_INFO, "MMU", "Added hole size %llu\n", region->base-memory_end);
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

        kdebugf(DEBUG_INFO, "MMU", "Region start=0x%llx, len=%llu\n", region->base, region->length);

        mmu_inc_zone_size(desired_zone_type, region->length);
        memory_end=region->base+region->length;
    }

    mmu_recompute_total_size();

    // Init frame allocator
    mmu_frame_init((u8*)start_addr, mmu_get_total_size());
    
    // Lock low memory
    mmu_reserve_low_memory();
    return 0;
}

void mmu_reserve_low_memory()
{
    // 0 - 0x4FF: IVT + BDA
    usize ivt_bda_pages = mmu_byte_to_4k_pages(0x500-0);
    mmu_frame_reserve_n(0, ivt_bda_pages);

    // 0x80000 - 0xA0000: EBDA
    usize ebda_pages = mmu_byte_to_4k_pages(0xA0000-0x80000);
    mmu_frame_reserve_n(0x80000, ebda_pages);
}