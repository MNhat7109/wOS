#pragma once
#include <stdarg.h>

extern void (*kputc)(char ch);
extern void (*kputs)(const char* str);
void kprintf(const char* fmt, ...);
void kvprintf(const char* fmt, va_list args);
