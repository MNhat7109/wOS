#pragma once
#include <stdarg.h>

extern void (*putchar)(char ch);
extern void (*puts)(const char* str);
void printf(const char* fmt, ...);
void vprintf(const char* fmt, va_list args);
