#pragma once
#include "vga/vga.h"

void kputc(char ch);
void kputs(const char* str);
void kprintf(const char* fmt, ...);
#define kclrscr(_c) clear((_c))