#pragma once
#include <libk/stdint.h>
#include <stdarg.h>

typedef struct stdio_ctx_t
{
    char* buf;
    usize pos, len, cnt;
} stdio_ctx_t;

void kputc(char ch);
void kputs(const char* str);
void ksnputc(stdio_ctx_t* ctx, char ch);
void ksnputs(stdio_ctx_t* ctx, const char* str);

void kprintf(const char* fmt, ...);
void kvprintf(const char* fmt, va_list arg);
void ksnprintf(char* buf, usize n, const char* fmt, ...);
void kvsnprintf(char* buf, usize n, const char* fmt, va_list arg);
void kclrscr();