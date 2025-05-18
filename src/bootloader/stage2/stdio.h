#pragma once
#include "vga/vga.h"
#include "vesa/vesa.h"

void kputc(char ch);
void kputs(const char* str);
void kprintf(const char* fmt, ...);
void kclrscr(u8 color);
void reset_cursor();
void save_cursor();