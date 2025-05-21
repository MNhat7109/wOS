#include "video.h"
#include "../string/string.h"

framebuffer_t screen_buffer;
font_t screen_font;
u32 cx, cy, pitch, pix_width; 
u8 fw, fh;
u32 sw, sh;
u8* glyph;

void video_init(framebuffer_t* fb, font_t* fon)
{
    screen_buffer = *fb;
    screen_font = *fon;
    cx = screen_buffer.xpos;
    cy = screen_buffer.ypos;
    sw = screen_buffer.width;
    sh = screen_buffer.height;
    pitch = screen_buffer.pitch;
    pix_width = screen_buffer.bpp>>3;
    fw = screen_font.width;
    fh = screen_font.height;
    glyph = (u8*)screen_font.glyph;
}

void video_putpixel(u32 x, u32 y, u32 color)
{
    u8* fb = (u8*)screen_buffer.base;
    fb[y*pitch+x*pix_width] = color&0xFF;
    fb[y*pitch+x*pix_width+1] = (color>>8)&0xFF;
    fb[y*pitch+x*pix_width+2] = (color>>16)&0xFF;
}

void video_clear(u32 color)
{
    for (int y=0;y<sh;y++)
    {
        for (int x=0;x<sw;x++)
            video_putpixel(x, y, color);
    }
}

void video_scroll(u32 lines)
{
    for (u32 i=0;i<sh-lines*fh;i++)
    {
        memmove((void*)screen_buffer.base+i*pitch, 
        (const void*)screen_buffer.base+(lines*fh+i)*pitch, pitch);
    }
    memset((void*)screen_buffer.base+(sh-lines*fh)*pitch, 0, pitch*fh);
}

void video_putch(u32 x, u32 y, char ch, u32 fg, u32 bg)
{
    u8* pix_buf = (u8*)screen_buffer.base;
    u8 char_size = fh;
    u8* char_glyph = glyph+ch*char_size;
    u32 glyph_offset = (y*fh*pitch)+(x*fw*pix_width);
    u32 bytes_per_line = (fw+7)>>3;
    u32 px, py, ln;
    for (py=0;py<fh;py++)
    {
        ln = glyph_offset;
        for (px=0;px<fw;px++)
        {
            *((u32*)(pix_buf+ln)) = char_glyph[px>>3]&(0x80>>(px&7))?fg:bg;
            ln+=pix_width;
        }
        char_glyph+=bytes_per_line;
        glyph_offset+=pitch;
    }
}