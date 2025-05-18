#include "vga.h"
#include "../io/io.h"

const u32 SW = 80, SH=25;
volatile char* vram = (volatile char*)0xb8000;

void putcolor(u32 x, u32 y, u8 fg, u8 bg)
{
    vram[2*(y*SW+x)+1] = (bg&0x7) << 4 | (fg&0xF);
}

void putcolor_internal(u32 x, u32 y, u8 color)
{
    vram[2*(y*SW+x)+1] = color;
}
void VGA_clear(u8 bg)
{
    for (u32 i =0 ;i<SH; i++)
    {
        for (u32 j=0;j<SW;j++)
            putcolor(j, i, bg, bg);
    }
}

void VGA_putch(u32 x, u32 y, char ch, u8 fg, u8 bg)
{
    vram[2*(y*SW+x)] = ch;
    putcolor(x,y,fg,bg);
}

void putch_internal(u32 x, u32 y, char ch, u8 color)
{
    vram[2*(y*SW+x)] = ch;
    putcolor_internal(x,y,color);
}


char VGA_getch(u32 x, u32 y)
{
    return vram[2*(y*SW+x)];
}

u8 VGA_getcolor(u32 x, u32 y)
{
    return vram[2*(y*SW+x)+1];
}

void VGA_scroll(int lines)
{
    u8 col = VGA_getcolor(0,0);
    for (int i = lines; i < SH; i++)
    {
        for (int j=0;j<SW;j++)
        {
            putch_internal(j, i, VGA_getch(j, i-lines), VGA_getcolor(j, i-lines));
        }
    }

    for (int i=SH-lines;i<SH;i++)
    {
        for (int j=0;j<SW;j++)
        {
            putch_internal(j, i, '\0', col);
        }
    }
}

void VGA_set_cursor(u32 x, u32 y)
{
    u32 pos = y*SW+x;
    // TODO: inb and outb
    outb(0x3D4, 0x0F); // Write to register 0x0F to set the highest 8 bits of cursor location.
    // Set the data on port 0x3D5
    outb(0x3D5, ((pos&0xF)>>8));
    outb(0x3D4, 0x10); // Same thing for lower 8 bits
    outb(0x3D5, (pos>>8));
}