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
} mmu_zone_type_t;

usize mmu_get_zone_size(int zone_type);
usize mmu_inc_zone_size(int zone_type, usize value);
usize mmu_dec_zone_size(int zone_type, usize value);

usize mmu_get_total_size();
void mmu_set_total_size(usize mem_size);