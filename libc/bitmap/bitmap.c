#include <bitmap.h>
#include <string.h>

void bitmap_init(bitmap_t* bmp, u8* buffer, usize bit_count)
{
    bmp->bit_count = bit_count;
    bmp->size = (bit_count+0x7)>>3;
    bmp->buffer = buffer;
    memset(bmp->buffer, 0, bmp->size);
}

void bitmap_set(bitmap_t* bmp, usize idx)
{
    usize buffer_idx = idx>>3;
    usize bit_idx = idx & 7;

    bmp->buffer[buffer_idx] |= (1<<bit_idx);
}

void bitmap_clear(bitmap_t* bmp, usize idx)
{
    usize buffer_idx = idx>>3;
    usize bit_idx = idx & 7;

    bmp->buffer[buffer_idx] &= ~(1<<bit_idx);
}

u8 bitmap_get(bitmap_t* bmp, usize idx)
{
    usize buffer_idx = idx>>3;
    usize bit_idx = idx & 7;

    return (bmp->buffer[buffer_idx] & (1<<bit_idx)) >> bit_idx;
}