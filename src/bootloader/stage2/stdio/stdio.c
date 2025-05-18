#include "../stdio.h"

unsigned xpos=0, ypos=0;

void kclrscr(u8 color)
{
    if (vesa_inited) VESA_clear(color);
    else VGA_clear(color);
}

void putch(u32 x, u32 y, char ch, u8 fg, u8 bg)
{
    if (vesa_inited) VESA_putch(x,y,ch,vga_to_24bpp_color_map[fg],
        vga_to_24bpp_color_map[bg]);
    else VGA_putch(x,y,ch, fg,bg);
}

void scroll(int lines)
{
    if (vesa_inited) VESA_scroll(lines);
    else VGA_scroll(lines);
}

void set_cursor(u32 x, u32 y)
{
    if (vesa_inited) return;
    VGA_set_cursor(x,y);
}

void reset_cursor()
{
    xpos=0; ypos=0;    
}

void save_cursor()
{
    fb_out.xpos = xpos;
    fb_out.ypos = ypos;
}

void kputc(char ch)
{
    switch (ch)
    {
        case '\0': break;
        case '\t':
            xpos+=4;
            break;
        case '\n':
            ypos++;
        case '\r':
            xpos=0;
            break;
        default:
            putch(xpos, ypos, ch, VGA_COLOR_GRAY, VGA_COLOR_BLACK);
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