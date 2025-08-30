#pragma once
#include <libk/stdint.h>

void page_alloc_init(void* bitmap_base);

void page_alloc_free(void* address);
void page_alloc_lock(void* address);

void page_alloc_freen(void* address, usize page_count);
void page_alloc_lockn(void* address, usize page_count);

void* page_alloc_request();

struct bitmap_t;
typedef struct bitmap_t bitmap_t;

bitmap_t const* page_get_bitmap();