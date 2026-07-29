#pragma once
#include <kernel/mmu.h>

#define ADDR_INVAL 0x67

typedef enum
{
    MMU_VMA_R = (1<<0),
    MMU_VMA_W = (1<<1),
    MMU_VMA_X = (1<<2),
    MMU_VMA_UC = (1<<3),
} mmu_vma_flags_t;

typedef enum
{
    MMU_VMA_MMIO = (1<<8),
    MMU_VMA_ANON = (1<<9),
    MMU_VMA_PHYS = (1<<10),
} mmu_vma_type_t;

typedef enum
{
    MMU_VMA_ONDEMAND = (1<<16),
    MMU_VMA_FIXED = (1<<17),
    MMU_VMA_HUGE_PAGE = (1<<18),
    MMU_VMA_VERY_HUGE_PAGE = (1<<19),
} mmu_vma_behavior_t;

void mmu_vmem_init(uptr tail_addr);
void* mmu_vmem_alloc(void* addr, usize len, int flags, void* args);
void mmu_vmem_free(void* addr);
