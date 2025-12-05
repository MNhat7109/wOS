#pragma once
#include "../stdint.h"

void VGA_clear(u8 bg);
void VGA_putch(u32 x, u32 y, char ch, u8 fg, u8 bg);
char VGA_getch(u32 x, u32 y);
u8   VGA_getcolor(u32 x, u32 y);
void VGA_scroll(int lines);
void VGA_set_cursor(u32 x, u32 y);
u32 VGA_get_SCH();
u32 VGA_get_SCW();