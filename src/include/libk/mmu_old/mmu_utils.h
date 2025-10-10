#pragma once
#include <libk/stdint.h>

extern usize vaddr_offset;

u64 mmu_get_total_memsize();

usize mmu_get_reserved_memsize();
usize mmu_get_free_memsize();
usize mmu_get_used_memsize();

usize mmu_log2(usize n);
usize mmu_ptov(usize paddr);
usize mmu_vtop(usize vaddr);

#define __va(_pt, _pa) (_pt)mmu_ptov((usize)(_pa))
#define __pa(_pt, _va) (_pt)mmu_vtop((usize)(_va))