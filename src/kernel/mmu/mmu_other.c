#include <kernel/mmu_other.h>
#include <kernel/mmu.h>
#include <kernel/debug.h>

static struct
{
    u64 addressable_size;
    u64 phys_span;
    u64 zone_size[7];
} mmu_zone_data;

u64 mmu_get_zone_size(int zone_type)
{
    return mmu_zone_data.zone_size[zone_type];
}

void mmu_inc_zone_size(int zone_type, usize value)
{
    // if (mmu_zone_data.zone_size[zone_type] >= mmu_zone_data.addressable_size) return;

    // usize left = mmu_zone_data.addressable_size - mmu_zone_data.zone_size[zone_type];
    // usize inc = value < left?left:value;
    mmu_zone_data.zone_size[zone_type] += value;
}

void mmu_dec_zone_size(int zone_type, usize value)
{
    // if (!mmu_zone_data.zone_size[zone_type]) return;
    
    // usize dec = value < mmu_zone_data.zone_size[zone_type]?
    // value:
    // mmu_zone_data.zone_size[zone_type];
    
    mmu_zone_data.zone_size[zone_type] -= value;
}

u64 mmu_get_total_size()
{
    return mmu_zone_data.phys_span;
}

void mmu_recompute_mem_size(memory_info_t* mem_map)
{
    // Make use of mem_map for basic memory size, and size of each category (used, free, etc.)
    u64 memory_end=0, highest = 0;
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

        memory_end=region->base+region->length;
        switch (region->type)
        {
            case MEMORY_TYPE_FREE:
                desired_zone_type = MMU_ZONE_FREE;
                highest = (highest < memory_end)?memory_end:highest;
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
    }

    for (int i=0;i<7;i++) 
    {
        kdebugf(DEBUG_INFO, "MMU", "Zone %d: %llu\n",i, mmu_zone_data.zone_size[i]);
        mmu_zone_data.addressable_size += mmu_zone_data.zone_size[i];
    }

    mmu_zone_data.phys_span = highest;
    kdebugf(DEBUG_INFO, "MMU", "Physical RAM span: %llu bytes\n", mmu_zone_data.phys_span);
}