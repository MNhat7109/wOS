#include "../../drivers/vesa.h"
#include "../../string/string.h"
#include "../../stdio.h"
#include "../../boot_info.h"

bool vesa_inited = false;

static struct
{
    framebuffer_t* fb;
    font_t* font;
    volatile u8* video_buffer;
} vesa_data;

void VESA_init(framebuffer_t* framebuffer, font_t* font)
{
    // Check if both the framebuffer (fb) and the font is supplied
    if (!framebuffer || !font) return;
    // Check fb and font's sanity
    if (!framebuffer->base || !font->glyph) return;
    vesa_data.fb = framebuffer;
    vesa_data.video_buffer = (u8*)vesa_data.fb->base;
    vesa_data.font = font;
    vesa_inited =true;
}

bool VESA_check_status()
{
    return vesa_inited;
}

void VESA_putpix(u32 x, u32 y, u32 color)
{
    vesa_data.video_buffer[y*vesa_data.fb->pitch+x*4] = color&0xFF;
    vesa_data.video_buffer[y*vesa_data.fb->pitch+x*4+1] = (color>>8)&0xFF;
    vesa_data.video_buffer[y*vesa_data.fb->pitch+x*4+2] = (color>>16)&0xFF;
}

void VESA_putch(u32 x, u32 y, char ch, u32 fg, u32 bg)
{
    u8 font_height = vesa_data.font->height, font_width = vesa_data.font->width, font_char_size = vesa_data.font->height;
    u8* glyph = (u8*)(vesa_data.font->glyph+ch*font_char_size);
    u32 offset = (y*font_height*vesa_data.fb->pitch)+(x*(font_width)*4);
    int bytes_per_line = (font_width+7)/8;
    int cx,cy,line;
    for (cy=0;cy<font_height;cy++)
    {
        line=offset;
        for (cx=0;cx<font_width;cx++)
        {
            *((u32*)(vesa_data.video_buffer+line)) = glyph[cx/8]&(0x80>>(cx&7))?fg:bg;
            line+=4;
        }
        glyph += bytes_per_line;
        offset += vesa_data.fb->pitch;
    }
}

void VESA_clear(u32 color)
{
    for (int y=0;y<vesa_data.fb->height;y++)
    {
        for (int x=0;x<vesa_data.fb->width;x++)
            VESA_putpix(x,y, color);
    }
}

void VESA_scroll(int lines)
{
    u32 font_height = vesa_data.font->height;
    int i;
    for (i=0;i<vesa_data.fb->height-lines*font_height;i++)
        memmove((void*)vesa_data.video_buffer+i*vesa_data.fb->pitch,
    (const void*)vesa_data.video_buffer+(lines*font_height+i)*vesa_data.fb->pitch,
vesa_data.fb->pitch);
    memset((void*)vesa_data.video_buffer+(vesa_data.fb->height-lines*font_height)*vesa_data.fb->pitch,
'\0', font_height*vesa_data.fb->pitch);
}

u32 VESA_get_SCH()
{
    return vesa_data.fb->height/vesa_data.font->height;
}

u32 VESA_get_SCW()
{
    return vesa_data.fb->width/vesa_data.font->width;
}