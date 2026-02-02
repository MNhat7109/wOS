#include <kernel/console.h>

void console_writechar(console_t* console, char ch);

void console_scroll(console_t* console, int lines)
{
    
}

void console_clear(console_t* console)
{
    console->backend->clear();
    // Clear ring
}

void console_writestr(console_t* console, const char* str)
{
    while (*str)
    {
        console_writechar(console, *str);
        str++;
    }
}