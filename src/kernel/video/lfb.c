#include <kernel/video.h>
#include <string.h>
#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <stdbool.h>

typedef struct font_t
{
    u8 height, width;
    u16 glyph_count;
    void* glyph; 
} __attribute__((packed)) font_t;

font_t* lfb_font = NULL;
volatile u8* fb_base;
u32 fb_height, fb_width, fb_pitch, fb_bpp;
extern u32 current_fg, current_bg;

void lfb_clrscr(u32 color);
void lfb_putpix(u32 x, u32 y, u32 color);
void lfb_putchar(char c, u32 cx, u32 cy, u32 fg, u32 bg);
void lfb_scroll(u32 lines);
void lfb_setcursor(u32 x, u32 y);

video_ops_t lfb_ops = {
    .clear = &lfb_clrscr,
    .putchar = &lfb_putchar,
    .putpix = &lfb_putpix,
    .scroll = &lfb_scroll,
    .set_cursor = &lfb_setcursor
};

int lfb_init(framebuffer_t* fb, font_t* font)
{
    if (!fb || !font) return -1;

    lfb_font = font;

    paddr_t font_phys = mmu_vtop((vaddr_t)font->glyph);
    usize glyph_pages = mmu_byte_to_4k_pages(font->glyph_count*font->width);
    mmu_frame_set_n(font_phys, glyph_pages);
    mmu_mmapn(font_phys, glyph_pages, 0, 0);

    usize fb_base_page_count = mmu_byte_to_4k_pages(fb->size);
    mmu_mmapn(fb->base, fb_base_page_count, MMU_PG_ATTR_PCD | MMU_PG_ATTR_RW, MMU_FLAG_MAP_ID);

    fb_base = (u32*)fb->base;
    fb_height = fb->height;
    fb_width = fb->width;
    fb_pitch = fb->pitch;
    fb_bpp = fb->bpp;

    current_fg = 0xFFFFFF;
    current_bg = 0;

    video_ops = &lfb_ops;
    return 0;
}

void lfb_clrscr(u32 color)
{
    for (u32 y=0;y<fb_height;y++)
    {
        for (u32 x=0;x<fb_width;x++)
            lfb_putpix(x,y,color);
    }
}

void lfb_putpix(u32 x, u32 y, u32 color)
{

    u8 bytes_per_pix = fb_bpp >> 3;

    u32 pos = y*fb_pitch+x*bytes_per_pix;
    fb_base[pos] = color&0xFF;
    fb_base[pos+1] = (color>>8)&0xFF;
    fb_base[pos+2] = (color>>16)&0xFF;
}

void lfb_putchar(char c, u32 cx, u32 cy, u32 fg, u32 bg)
{

    u8 bytes_per_pix = fb_bpp >> 3;
    u8* char_glyph = (u8*)(lfb_font->glyph+c*lfb_font->height);
    u32 bytes_per_line = (lfb_font->width+0x7)>>3;
    
    for (u32 pcy=0;pcy<lfb_font->height;pcy++)
    {
        for (u32 pcx=0;pcx<lfb_font->width;pcx++)
        {
            u32 glyph_offset_bytes = pcx>>3;
            u32 glyph_offset_bits = pcx&0x7;
            u8 bit_set = 
            (char_glyph+pcy*bytes_per_line)[glyph_offset_bytes]
            &(0x80>>glyph_offset_bits);
            lfb_putpix(
                (pcx+cx*lfb_font->width), 
                pcy+cy*lfb_font->height,
                bit_set?fg:bg
            );
        }
    }

}

void lfb_scroll(u32 lines)
{
    u32 font_height = lfb_font->height;
    int i;
    for (i=0;i<fb_height-lines*font_height;i++)
        memmove((void*)fb_base+i*fb_pitch,
    (const void*)fb_base+(lines*font_height+i)*fb_pitch,
fb_pitch);
    memset((void*)fb_base+(fb_height-lines*font_height)*fb_pitch,
'\0', font_height*fb_pitch);
}

void lfb_setcursor(u32 x, u32 y)
{

}