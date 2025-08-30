#include <libk/video/video.h>
#include <libk/string.h>

framebuffer_t screen_buffer;
font_t screen_font;
usize cx, cy, pitch, pix_width; 
u8 fw, fh;
usize sw, sh;
u8* glyph;
usize char_sw, char_sh;

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

    char_sw = sw/fw; char_sh = sh/fh;
}

void video_putpixel(usize x, usize y, usize color)
{
    u8* fb = (u8*)screen_buffer.base;
    fb[y*pitch+x*pix_width] = color&0xFF;
    fb[y*pitch+x*pix_width+1] = (color>>8)&0xFF;
    fb[y*pitch+x*pix_width+2] = (color>>16)&0xFF;
}

void video_clear(usize color)
{
    for (int y=0;y<sh;y++)
    {
        for (int x=0;x<sw;x++)
            video_putpixel(x, y, color);
    }
}

void video_scroll(usize lines)
{
    for (usize i=0;i<sh-lines*fh;i++)
    {
        memmove((void*)screen_buffer.base+i*pitch, 
        (const void*)screen_buffer.base+(lines*fh+i)*pitch, pitch);
    }
    memset((void*)screen_buffer.base+(sh-lines*fh)*pitch, 0, pitch*fh);
}

void video_putch(usize x, usize y, char ch, usize fg, usize bg)
{
    u8* pix_buf = (u8*)screen_buffer.base;
    u8 char_size = fh;
    u8* char_glyph = glyph+ch*char_size;
    usize glyph_offset = (y*fh*pitch)+(x*fw*pix_width);
    usize bytes_per_line = (fw+7)>>3;
    usize px, py, ln;
    for (py=0;py<fh;py++)
    {
        ln = glyph_offset;
        for (px=0;px<fw;px++)
        {
            *((usize*)(pix_buf+ln)) = char_glyph[px>>3]&(0x80>>(px&7))?fg:bg;
            ln+=pix_width;
        }
        char_glyph+=bytes_per_line;
        glyph_offset+=pitch;
    }
}