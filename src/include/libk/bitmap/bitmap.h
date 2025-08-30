#pragma once
#include <libk/stdint.h>

typedef struct bitmap_t
{
    u64 size;
    u8* buffer;
} bitmap_t;

void bitmap_init(bitmap_t* new_map, u64 size, void* buffer);
u8 bitmap_get_bits(bitmap_t* bmp, u64 index);
void bitmap_set_bits(bitmap_t* bmp, u64 index);
void bitmap_clear_bits(bitmap_t* bmp, u64 index);