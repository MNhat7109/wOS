#pragma once
#include <libk/stdint.h>

void buddy_alloc_init(void* address, usize page_count);
void buddy_alloc_destroy();
void* buddy_alloc_request(usize size);
void buddy_alloc_free(void* ptr);