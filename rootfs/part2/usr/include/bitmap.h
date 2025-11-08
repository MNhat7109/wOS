#pragma once
#include <stdint.h>

typedef struct bitmap_t 
{
    usize bit_count;
    usize size;
    u8* buffer;
} bitmap_t;

void bitmap_init(bitmap_t* bmp, uptr address, usize bit_count);

void bitmap_set(bitmap_t* bmp, usize idx);
u8 bitmap_get(bitmap_t* bmp, usize idx);