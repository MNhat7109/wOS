#include "vga.h"
#include "../string/string.h"
#include "../io/io.h"

const u32 SW = 80, SH=25;
volatile char* vram = (volatile char*)0xb8000;

void clear(u8 bg)
{
    for (u32 i =0 ;i<SH; i++)
    {
        for (u32 j=0;j<SW;j++)
            putcolor(j, i, bg, bg);
    }
}

void putch(u32 x, u32 y, char ch)
{
    vram[2*(y*SW+x)] = ch;
}

void putcolor(u32 x, u32 y, u8 fg, u8 bg)
{
    vram[2*(y*SW+x)+1] = (bg&0x7) << 4 | (fg&0xF);
}

void putcolor_internal(u32 x, u32 y, u8 color)
{
    vram[2*(y*SW+x)+1] = color;
}

char getch(u32 x, u32 y)
{
    return vram[2*(y*SW+x)];
}

u8 getcolor(u32 x, u32 y)
{
    return vram[2*(y*SW+x)+1];
}

void scroll(int lines)
{
    u8 col = getcolor(0,0);
    for (int i = lines; i < SH; i++)
    {
        for (int j=0;j<SW;j++)
        {
            putcolor_internal(j, i, getcolor(j, i-lines));
            putch(j, i, getch(j, i-lines));
        }
    }

    for (int i=SH-lines;i<SH;i++)
    {
        for (int j=0;j<SW;j++)
        {
            putcolor_internal(j, i, col);
            putch(j, i, '\0');
        }
    }
}

void set_cursor(u32 x, u32 y)
{
    u32 pos = y*SW+x;
    // TODO: inb and outb
    outb(0x3D4, 0x0F); // Write to register 0x0F to set the highest 8 bits of cursor location.
    // Set the data on port 0x3D5
    outb(0x3D5, ((pos&0xF)>>8));
    outb(0x3D4, 0x10); // Same thing for lower 8 bits
    outb(0x3D5, (pos>>8));
}