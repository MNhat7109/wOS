#include <libk/bitmap/bitmap.h>

void bitmap_init(bitmap_t* new_map, u64 size, void* buffer)
{
    new_map->size = size >> 3; // We initialize a bitmap with the size of n bits, not bytes
    if (size&7) new_map->size++;
    
    new_map->buffer = (u8*)buffer;
    for (u64 i=0; i<new_map->size; i++) new_map->buffer[i] = 0;
}

u8 bitmap_get_bits(bitmap_t* bmp, u64 index)
{
    u64 byte_idx = index >> 3;
    u64 bit_idx = index & 7;
    u8 mask = 1 << bit_idx;
    return (bmp->buffer[byte_idx] & mask) >> bit_idx; 
}

void bitmap_set_bits(bitmap_t* bmp, u64 index)
{
    u64 byte_idx = index >> 3;
    u64 bit_idx = index & 7;
    u8 mask = 1 << bit_idx;
    bmp->buffer[byte_idx] |= mask;
}

void bitmap_clear_bits(bitmap_t* bmp, u64 index)
{
    u64 byte_idx = index >> 3;
    u64 bit_idx = index & 7;
    u8 mask = 1 << bit_idx;
    bmp->buffer[byte_idx] &= ~mask;
}

