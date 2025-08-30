#pragma once
#include <libk/stdint.h>

void slab_alloc_init(void* address, usize page_count);
void slab_alloc_destroy();

usize slab_alloc_get_max_ptr_size();

void* slab_alloc_request(usize size);
void slab_alloc_free(void* ptr);