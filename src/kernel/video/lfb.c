#include <kernel/console.h>
#include <string.h>
#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <stdbool.h>

typedef struct framebuffer_t
{
    u32 base;
    u32 size;
    u32 height, width;
    u32 bpp, pitch;
    u32 xpos, ypos;
} __attribute__((packed)) framebuffer_t;

typedef struct font_t
{
    u8 height, width;
    u16 glyph_count;
    void* glyph; 
} __attribute__((packed)) font_t;

void lfb_clrscr();
void lfb_putpix(u32 x, u32 y, u32 color);
void lfb_putchar(char c);
void lfb_setcursor(u32 x, u32 y);
void lfb_setcolor(u32 fg, u32 bg);

console_backend_t lfb_backend = {
    .clear = &lfb_clrscr,
    .putchar = &lfb_putchar,
    .set_cursor = &lfb_setcursor,
    .set_color = &lfb_setcolor
};

extern console_backend_t* video_backend;
extern u32 video_cols, video_rows, video_fg, video_bg;

volatile u8* fb_base;
u32 fb_height, fb_width, fb_pitch, fb_bpp, fb_bytes_per_pix;
u32 font_height, font_width, font_glyph_count;
void* font_glyph;

int lfb_init(framebuffer_t* fb, font_t* font)
{
    if (!fb || !font) return -1;

    paddr_t font_phys = mmu_vtop((vaddr_t)font->glyph);
    usize glyph_pages = mmu_byte_to_4k_pages(font->glyph_count*font->width);
    mmu_frame_set_n(font_phys, glyph_pages);
    mmu_mmapn(font_phys, glyph_pages, 0, 0);

    usize fb_base_page_count = mmu_byte_to_4k_pages(fb->size);
    mmu_mmapn(fb->base, fb_base_page_count, MMU_PG_ATTR_PCD | MMU_PG_ATTR_RW, MMU_FLAG_MAP_ID);

    fb_base = (volatile u8*)fb->base;
    fb_height = fb->height;
    fb_width = fb->width;
    fb_pitch = fb->pitch;
    fb_bpp = fb->bpp;
    fb_bytes_per_pix = fb_bpp >> 3;

    font_height = font->height;
    font_width = font->width;
    font_glyph_count = font->glyph_count;
    font_glyph = font->glyph;

    video_backend = &lfb_backend;
    return 0;
}

void lfb_clrscr()
{
    for (u32 y=0;y<fb_height;y++)
    {
        for (u32 x=0;x<fb_width;x++)
            lfb_putpix(x,y,video_bg);
    }
}

void lfb_putpix(u32 x, u32 y, u32 color)
{
    u32 pos = y*fb_pitch+x*fb_bytes_per_pix;
    fb_base[pos] = color&0xFF;
    fb_base[pos+1] = (color>>8)&0xFF;
    fb_base[pos+2] = (color>>16)&0xFF;
}

void lfb_putchar(char c)
{
    u8* char_glyph = (u8*)(font_glyph+c*font_height);
    u32 bytes_per_line = (font_width+0x7)>>3;
    
    for (u32 pcy=0;pcy<font_height;pcy++)
    {
        for (u32 pcx=0;pcx<font_width;pcx++)
        {
            u32 glyph_offset_bytes = pcx>>3;
            u32 glyph_offset_bits = pcx&0x7;
            u8 bit_set = 
            (char_glyph+pcy*bytes_per_line)[glyph_offset_bytes]
            &(0x80>>glyph_offset_bits);
            lfb_putpix(
                (pcx+video_cols*font_width), 
                pcy+video_rows*font_height,
                bit_set?video_fg:video_bg
            );
        }
    }

}

// void lfb_scroll(u32 lines)
// {
//     u32 font_height = font_height;
//     int i;
//     for (i=0;i<fb_height-lines*font_height;i++)
//         memmove((void*)fb_base+i*fb_pitch,
//     (const void*)fb_base+(lines*font_height+i)*fb_pitch,
// fb_pitch);
//     memset((void*)fb_base+(fb_height-lines*font_height)*fb_pitch,
// '\0', font_height*fb_pitch);
// }

void lfb_setcursor(u32 cols, u32 rows)
{
    video_cols = cols;
    video_rows = rows;
}

void lfb_setcolor(u32 fg, u32 bg)
{
    video_fg = fg;
    video_bg = bg;
}