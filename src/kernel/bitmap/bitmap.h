#pragma once
#include "../stdint.h"

typedef struct
{
    u32 size;
    u8* buffer;
} __attribute__((packed)) bitmap_t;

void bitmap_init_buffer(bitmap_t* new_map, u32 size, u32 buffer);
void bitmap_init(bitmap_t* new_map, u32 size);
u8 bitmap_get_bits(bitmap_t* bmp, u32 index);
void bitmap_set_bits(bitmap_t* bmp, u32 index);
void bitmap_clear_bits(bitmap_t* bmp, u32 index);