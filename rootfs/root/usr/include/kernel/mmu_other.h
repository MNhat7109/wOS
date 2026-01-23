#pragma once
#include <stdint.h>

typedef enum
{
    MMU_ZONE_FREE,
    MMU_ZONE_USED,
    MMU_ZONE_HW_RESERVED,
    MMU_ZONE_OS_RESERVED,
    MMU_ZONE_HOLE,
    MMU_ZONE_BAD,
    MMU_ZONE_OTHER,
} mmu_zone_type_t;

u64 mmu_get_zone_size(int zone_type);
void mmu_inc_zone_size(int zone_type, usize value);
void mmu_dec_zone_size(int zone_type, usize value);

u64 mmu_get_total_size();
void mmu_recompute_total_size();