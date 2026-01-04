#include "../stdio.h"
#include "../stdint.h"
#include "../drivers/console.h"

unsigned xpos=0, ypos=0;

void kclrscr(u8 color)
{
    console_write("\x1B[2J");
}

void kputc(char ch)
{
    console_putchar(ch);
}

void kputs(const char* str)
{
    console_write(str);
}

void kdebug_besilent()
{
    console_switch_mode(CONSOLE_MODE_SERIAL);
}

void kdebug_benoisy()
{
    console_switch_mode(CONSOLE_MODE_BOTH);
}