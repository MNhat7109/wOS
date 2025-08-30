#pragma once
#include <libk/stdint.h>

void mmu_init_heap(void* address, usize page_count);
void mmu_destroy_heap();

void* mmu_allocate(usize size);
void* mmu_reallocate(void* ptr, usize size);
void mmu_free(void* block);