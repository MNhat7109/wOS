#pragma once
#include <libk/stdint.h>

typedef struct
{
    usize base;
    usize size;
    usize height, width;
    usize bpp, pitch;
    usize xpos, ypos;
} __attribute__((packed)) framebuffer_t;

typedef struct
{
    u8 height, width;
    u16 glyph_count;
    void* glyph;
} __attribute__((packed)) font_t;

extern usize cx, cy, sw, sh;
extern u8 fw, fh;

extern usize char_sw, char_sh;

void video_init(framebuffer_t* fb, font_t* fon);
void video_putpixel(usize x, usize y, usize color);
void video_clear(usize color);
void video_scroll(usize lines);
void video_putch(usize x, usize y, char ch, usize fg, usize bg);