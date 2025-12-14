#pragma once
#include "../boot_info.h"

typedef enum
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_PURPLE,
    VGA_COLOR_BROWN,
    VGA_COLOR_GRAY,
    VGA_COLOR_DARK_GRAY,
    VGA_COLOR_LIGHT_BLUE,
    VGA_COLOR_LIGHT_GREEN,
    VGA_COLOR_LIGHT_CYAN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_PURPLE,
    VGA_COLOR_YELLOW,
    VGA_COLOR_WHITE,
} legacy_vga_color_t;

extern u32 video_vga_to_24bpp_color_map[16];

void video_init(boot_info_t* boot_info);
void video_putch(char ch, u32 x, u32 y);
void video_setcolor(u8 fg, u8 bg);
void video_getcolor(u8* fg_out, u8* bg_out);
void video_scroll(u32 lines);
void video_clrscr();
void video_setcursor(u32 x, u32 y);
u32 video_get_SCH();
u32 video_get_SCW();