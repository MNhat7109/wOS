#include <kernel/mmu_other.h>
#include <kernel/debug.h>

static struct
{
    u64 total_size;
    u64 zone_size[7];
} mmu_zone_data;

u64 mmu_get_zone_size(int zone_type)
{
    return mmu_zone_data.zone_size[zone_type];
}

void mmu_inc_zone_size(int zone_type, usize value)
{
    // if (mmu_zone_data.zone_size[zone_type] >= mmu_zone_data.total_size) return;

    // usize left = mmu_zone_data.total_size - mmu_zone_data.zone_size[zone_type];
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
    return mmu_zone_data.total_size;
}

void mmu_recompute_total_size()
{
    for (int i=0;i<7;i++) 
    {
            kdebugf(DEBUG_INFO, "MMU", "Zone %d: %llu\n",i, mmu_zone_data.zone_size[i]);
        mmu_zone_data.total_size += mmu_zone_data.zone_size[i];
    }
            kdebugf(DEBUG_INFO, "MMU", "Total: %llu\n", mmu_zone_data.total_size/1024/1024);
}