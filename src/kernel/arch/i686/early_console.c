#include <kernel/video.h>

void debug_console_putch(char ch);
void debug_console_write(const char* str);

void simple_console_putchar(char ch)
{
    video_putchar(ch);
    debug_console_putch(ch);
}

void simple_console_puts(char ch)
{
    video_puts(ch);
    debug_console_write(ch);
}