#pragma once
#include "../stdint.h"

enum vga_color
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
};

void VGA_clear(u8 bg);
void VGA_putch(u32 x, u32 y, char ch, u8 fg, u8 bg);
char VGA_getch(u32 x, u32 y);
u8   VGA_getcolor(u32 x, u32 y);
void VGA_scroll(int lines);
void VGA_set_cursor(u32 x, u32 y);

extern volatile char* vram;
extern const u32 SW;
extern const u32 SH;