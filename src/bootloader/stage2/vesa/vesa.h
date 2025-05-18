#pragma once
#include <stdbool.h>
#include "../stdint.h"

typedef struct
{
    u32 base;
    u32 size;
    u32 height, width;
    u32 bpp, pitch;
    u32 xpos, ypos;
} __attribute__((packed)) framebuffer_t;

extern volatile framebuffer_t fb_out;
extern bool vesa_inited;
extern u32 vga_to_24bpp_color_map[16];
bool VESA_init();
u16 VESA_scan_mode(u32 x, u32 y, u32 d);

void VESA_putpix(u32 x, u32 y, u32 color);
void VESA_clear(u32 color);
void VESA_putch(u32 x, u32 y, char ch, u32 fg, u32 bg);
void VESA_scroll(int lines);