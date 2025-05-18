#pragma once
#include "../stdint.h"

typedef struct
{
    u32 base;
    u32 size;
    u32 height, width;
    u32 bpp, pitch;
    u32 xpos, ypos;
} __attribute__((packed)) framebuffer_t;

typedef struct
{
    u8 height, width;
    u16 glyph_count;
    void* glyph;
} __attribute__((packed)) font_t;
