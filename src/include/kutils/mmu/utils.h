#pragma once
#include <libk/stdint.h>

usize mmu_klog2(usize num);
usize mmu_find_next_po2(usize num);
usize mmu_find_prev_po2(usize num);