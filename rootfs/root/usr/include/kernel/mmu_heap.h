#pragma once
#include <stdint.h>

int mmu_heap_init(void* starting_addr, usize starting_size);

void* mmu_heap_alloc(usize size);
void mmu_heap_free(void* ptr);