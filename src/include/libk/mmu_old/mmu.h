#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

typedef enum
{
    MMU_PA_FLAG_PRESENT = (1ULL<<0), // Present
    MMU_PA_FLAG_RW = (1ULL<<1), // Read-write
    MMU_PA_FLAG_US = (1ULL<<2), // User-super
    MMU_PA_FLAG_PWT = (1ULL<<3), // Write-through
    MMU_PA_FLAG_PCD = (1ULL<<4), // Uncacheable
    MMU_PA_FLAG_ACCESSED = (1ULL<<5), // Accessed indicator
    MMU_PA_FLAG_GLOBAL = (1ULL<<6), // No invalidate
    MMU_PA_FLAG_PSE = (1ULL<<7), // Huge page
    MMU_PA_FLAG_PAT = (1ULL<<12), // Page attribute table
    MMU_PA_FLAG_NX = (1ULL<<63), // Execute disable
} mmu_page_attr_flags_t;

typedef enum
{
    MMU_HUGE_PAGE_1G = 0,
    MMU_HUGE_PAGE_4M = 1,
    MMU_HUGE_PAGE_2M = 1,
} mmu_huge_page_flags_t;

#define mmu_byte_to_4k_pages(_n) (((_n) >> 12)+((_n)&((1<<12)-1)))
#define mmu_byte_to_4m_pages(_n) (((_n) >> 22)+((_n)&((1<<22)-1)))
#define mmu_byte_to_2m_pages(_n) (((_n) >> 21)+((_n)&((1<<21)-1)))
#define mmu_byte_to_1g_pages(_n) (((_n) >> 30)+((_n)&((1<<30)-1)))

u64 mmu_search_memrange(
    usize paddr_start, usize size, 
    usize step, u64 flags,
    bool (*criterion)(usize paddr)
);

u32 mmu_get_page_size();
u32 mmu_get_huge_page_size(int flags);

void mmu_mmap(u64 vaddr, u64 paddr, u64 attributes);
void mmu_mmapn(u64 addr, u64 offset, u64 n, u64 attributes);
void mmu_mmap_huge(u64 vaddr, u64 paddr, u64 attributes, int flags);
void mmu_mmapn_huge(u64 addr, u64 offset, u64 n, u64 attributes, int flags);

u64 mmu_munmap(u64 vaddr);
u64 mmu_munmapn(u64 vaddr, u64* n);
