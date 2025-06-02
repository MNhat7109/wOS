#pragma once
#include "../stdint.h"
#include "../bitmap/bitmap.h"

extern bitmap_t page_bitmap;

void page_alloc_init();

void page_alloc_free(u32 address);
void page_alloc_lock(u32 address);

void page_alloc_freen(u32 address, u32 page_count);
void page_alloc_lockn(u32 address, u32 page_count);

u32 page_alloc_request();
u32 page_convert_from_bytes(u32 size_bytes);

const u32 page_get_reserved_mem();
const u32 page_get_used_mem();
const u32 page_get_free_mem();