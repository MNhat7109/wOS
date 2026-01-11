#include <kernel/mmu_other.h>

static struct
{
    usize total_size;
    usize zone_size[7];
} mmu_zone_data;

usize mmu_get_zone_size(int zone_type)
{
    return mmu_zone_data.zone_size[zone_type];
}

void mmu_inc_zone_size(int zone_type, usize value)
{
    if (mmu_zone_data.zone_size[zone_type] >= mmu_zone_data.total_size) return;

    usize left = mmu_zone_data.total_size - mmu_zone_data.zone_size[zone_type];
    usize inc = value < left?left:value;
    mmu_zone_data.zone_size[zone_type] += inc;
}

void mmu_dec_zone_size(int zone_type, usize value)
{
    if (!mmu_zone_data.zone_size[zone_type]) return;
    
    usize dec = value < mmu_zone_data.zone_size[zone_type]?
    value:
    mmu_zone_data.zone_size[zone_type];
    
    mmu_zone_data.zone_size[zone_type] -= dec;
}

usize mmu_get_total_size()
{
    return mmu_zone_data.total_size;
}

void mmu_set_total_size(usize mem_size)
{
    mmu_zone_data.total_size = mem_size;
    mmu_zone_data.zone_size[MMU_ZONE_FREE] = mem_size;
}