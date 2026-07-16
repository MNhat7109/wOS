#pragma once
#include <kernel/mmu.h>

int mmu_map_page(vaddr_t vaddr, paddr_t paddr, u64 attributes);
int mmu_map_page_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);
int mmu_map_page_very_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);
int mmu_unmap_page(vaddr_t vaddr);

paddr_t mmu_walk_page_table(vaddr_t vaddr);

// void mmu_map_n_pages_krnl(paddr_t addr, usize n, u64 attributes, int flags);
// void mmu_map_n_pages_id(paddr_t addr, usize n, u64 attributes, int flags);
// usize mmu_unmap_n_pages(vaddr_t vaddr, usize n);
