#include <kernel/debug.h>
#include <stdio.h>
#include <stdarg.h>


static const char* str_debugmode[] = {
    "INFO",
    "WARN",
    "CRITICAL"
};

void kdebugf(int mode, const char* module, const char* fmt, ...)
{
    printf("'%s' [%s]: ", str_debugmode[mode], module);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    if (mode == DEBUG_CRITICAL) 
    {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}