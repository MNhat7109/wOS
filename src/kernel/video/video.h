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

extern u32 cx, cy, sw, sh;

void video_init(framebuffer_t* fb, font_t* fon);
void video_putpixel(u32 x, u32 y, u32 color);
void video_clear(u32 color);
void video_scroll(u32 lines);
void video_putch(u32 x, u32 y, char ch, u32 fg, u32 bg);