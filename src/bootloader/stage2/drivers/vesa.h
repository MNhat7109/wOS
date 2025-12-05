#pragma once
#include <stdbool.h>
#include "../stdint.h"

typedef struct framebuffer_t framebuffer_t;
typedef struct font_t font_t;
void VESA_init(framebuffer_t* framebuffer, font_t* font);
bool VESA_check_status();
void VESA_putpix(u32 x, u32 y, u32 color);
void VESA_clear(u32 color);
void VESA_putch(u32 x, u32 y, char ch, u32 fg, u32 bg);
void VESA_scroll(int lines);
u32 VESA_get_SCH();
u32 VESA_get_SCW();