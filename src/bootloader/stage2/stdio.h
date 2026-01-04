#pragma once
#include "stdint.h"
#include <stdarg.h>

typedef enum
{
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_CRITICAL,
} KPRINTF_MODE;

void kputc(char ch);
void kputs(const char* str);
void kprintf(const char* fmt, ...);
void kvprintf(const char* fmt, va_list args);
void kdebugf(int mode, const char* module, const char* fmt, ...);
void kdebugf_silent(int mode, const char* module, const char* fmt, ...);
void kclrscr(u8 color);
void reset_cursor();
void save_cursor();