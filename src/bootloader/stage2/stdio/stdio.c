#include "../stdio.h"

unsigned xpos=0, ypos=0;


void kputc(char ch)
{
    switch (ch)
    {
        case '\t':
            xpos+=4;
            break;
        case '\n':
            ypos++;
        case '\r':
            xpos=0;
            break;
        default:
            putch(xpos, ypos, ch);
            putcolor(xpos, ypos, 7,0);
            xpos++;
            break;
    }
    if (xpos > SW)
    {
        xpos=0;
        ypos++;
    }
    if (ypos > SH) 
    {
        scroll(1);
        ypos--;
    }

    set_cursor(xpos, ypos);
}
void kputs(const char* str)
{
    while (*str)
    {
        kputc(*str);
        str++;
    }
}