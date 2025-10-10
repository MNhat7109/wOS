#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

typedef enum
{
    MMU_PAGING_PRESENT = (1<<0),
    MMU_PAGING_PAE = (1<<1),
} mmu_paging_feature_flags_t;

void mmu_paging_init(void* addr);
void mmu_paging_enable();
u32 mmu_paging_get_features();
void* mmu_paging_get_addr();