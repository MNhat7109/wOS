#include "vesa.h"
#include "../x86/x86.h"
#include "../string/string.h"
#include "../stdio.h"
#include "font.h"

#define LINEAR2SEGOFF(_l) ((((_l)>>4)&0xFFFF)|((_l)&0xF))
#define SEGOFF2LINEAR(_so) (((_so)&0xFFFF)|((_so)>>12))
#define VESA_VBE_get_info(_a) _x86_VBE_get_info(LINEAR2SEGOFF((_a)))
#define VESA_VBE_get_mode(_a, _m) _x86_VBE_get_mode(LINEAR2SEGOFF((_a)), (_m))
#define VESA_VBE_set_mode(_m) _x86_VBE_set_mode((_m))
#define abs(_a) (((_a)<0)?-(_a):(_a))

bool vesa_inited = false;

u32 vga_to_24bpp_color_map[16] =
{
    0x000000,
    0x0000AA,
    0x00AA00,
    0x00AAAA,
    0xAA0000,
    0xAA00AA,
    0xAA5500,
    0xAAAAAA,
    0x555555,
    0x5555FF,
    0x55FF55,
    0x55FFFF,
    0xFF5555,
    0xFF55FF,
    0xFFFF55,
    0xFFFFFF
};

typedef struct
{
    char signature[4];
    u16 version;
    u16 oem[2];
    u8 capabilities[4];
    u32 video_mode_offset;
    u16 total_memory;
    u8 _reserved[492];
} __attribute__((packed)) VESA_VBE_info_t;

typedef struct
{
    u16 attributes;
    u8 window_a;
    u8 window_b;
    u16 granularity;
    u16 window_size;
    u16 segment_a;
    u16 segment_b;
    u32 win_func_ptr;
    u16 pitch; // Needed
    u16 width; // Needed
    u16 height; // Needed
    u8 x_char;
    u8 y_char;
    u8 planes;
    u8 bpp; // Needed
    u8 banks;
    u8 memory_model;
    u8 bank_size;
    u8 image_pages;
    u8 _reserved0;

    u8 red_mask_size;
    u8 red_field_pos;
    u8 green_mask_size;
    u8 green_field_pos;
    u8 blue_mask_size;
    u8 blue_field_pos;
    u8 reserved_mask_size;
    u8 reserved_field_pos;
    u8 direct_color_mode;

    u32 framebuffer; // needed
    u32 off_screen_offset; // needed
    u32 off_screen_size; // needed
    u8 _reserved1[206];
} __attribute__((packed)) VESA_VBE_mode_info_t;


VESA_VBE_info_t* vbe_info = (VESA_VBE_info_t*)0x30000;
VESA_VBE_mode_info_t* vbe_mode_info = (VESA_VBE_mode_info_t*)0x31000;
volatile char* backup_vram = (volatile char*)0x40000;
volatile framebuffer_t fb_out;

void copy_to_fb()
{
    for (int y=0;y<SH;y++)
    {
        memcpy(backup_vram, vram, SW*SH*2);
        for (int x=0;x<SW;x++)
        {
            u8 color = backup_vram[2*(y*SW+x)+1];
        }
    }
}

bool VESA_init()
{
    memcpy(vbe_info->signature, "VBE2", 4);
    bool status = VESA_VBE_get_info((u32)vbe_info);
    if (!status) return status;

    if (memcmp(vbe_info->signature, "VESA", 4)!=0)
    {
        kprintf("VESA: Invalid VESA VBE structure");
        return false;
    }

    u16 mode = VESA_scan_mode(1280, 800, 32);

    // TODO: Copy everything from the vram to the framebuffer
    copy_to_fb();
    VESA_VBE_set_mode((mode|0x4000));
    vesa_inited = true;
    reset_cursor();
    for (int y=0;y<3;y++)
        for (int x=0;x<SW;x++)
            kputc(backup_vram[2*(y*SW+x)]);
    fb_out.base = vbe_mode_info->framebuffer;
    fb_out.height = vbe_mode_info->height;
    fb_out.width = vbe_mode_info->width;
    fb_out.bpp = 32;
    fb_out.size = fb_out.height*fb_out.width*(fb_out.bpp/8);
    fb_out.pitch = vbe_mode_info->pitch;
    kprintf("\n");
    //VESA_putch(0,0,'a', vga_to_24bpp_color_map[VGA_COLOR_GRAY], vga_to_24bpp_color_map[VGA_COLOR_BLACK]);
    return true;
}

u16 VESA_scan_mode(u32 x, u32 y, u32 d)
{
    u16* modes = (u16*)SEGOFF2LINEAR(vbe_info->video_mode_offset);

    int ideal_mode = 0x13;
    int pix_diff, ideal_pix_diff = abs((320*200)-(x*y));
    int depth_diff, ideal_depth_diff = (8>=d)?8-d:(d-8)*2;
    for (int i =0;modes[i]!=0xFFFF;i++)
    {
        bool status = VESA_VBE_get_mode((u32)vbe_mode_info, modes[i]);
        if (!status) continue;

        // Check for linear framebuffer support
        u16 lfb_support = vbe_mode_info->attributes & 0x90;
        if (lfb_support != 0x90) continue;

        // Check for packed or direct color mode
        if (vbe_mode_info->memory_model != 4 && vbe_mode_info->memory_model != 6)
            continue;
        
        if (x == vbe_mode_info->width && y == vbe_mode_info->height
        && d == vbe_mode_info->bpp) return modes[i];

        pix_diff = abs((vbe_mode_info->width*vbe_mode_info->height)-(x*y));
        depth_diff = vbe_mode_info->bpp >= d? vbe_mode_info->bpp-d:(d-vbe_mode_info->bpp)*2;

        if (ideal_depth_diff > pix_diff 
            || (ideal_depth_diff == pix_diff && ideal_depth_diff > depth_diff))
        {
            ideal_mode = modes[i];
            ideal_pix_diff = pix_diff;
            ideal_depth_diff = depth_diff;
        }
    }
    return ideal_mode;
}

void VESA_putpix(u32 x, u32 y, u32 color)
{
    u8* fb = (u8*)vbe_mode_info->framebuffer;
    fb[y*vbe_mode_info->pitch+x*4] = color&0xFF;
    fb[y*vbe_mode_info->pitch+x*4+1] = (color>>8)&0xFF;
    fb[y*vbe_mode_info->pitch+x*4+2] = (color>>16)&0xFF;
}

void VESA_putch(u32 x, u32 y, char ch, u32 fg, u32 bg)
{
    u8* fb = (u8*)vbe_mode_info->framebuffer;
    u8 font_height = font.height, font_width = font.width, font_char_size = font.height;
    u8* glyph = (u8*)font.glyph+ch*font_char_size;
    u32 offset = (y*font_height*vbe_mode_info->pitch)+(x*(font_width)*4);
    int bytes_per_line = (font_width+7)/8;
    int cx,cy,line;
    for (cy=0;cy<font_height;cy++)
    {
        line=offset;
        for (cx=0;cx<font_width;cx++)
        {
            *((u32*)(fb+line)) = glyph[cx/8]&(0x80>>(cx&7))?fg:bg;
            line+=4;
        }
        glyph += bytes_per_line;
        offset += vbe_mode_info->pitch;
    }
}

void VESA_clear(u32 color)
{
    for (int y=0;y<vbe_mode_info->height;y++)
    {
        for (int x=0;x<vbe_mode_info->width;x++)
            VESA_putpix(x,y, color);
    }
}

void VESA_scroll(int lines)
{
    u32 font_height = font.height;
    int i;
    for (i=0;i<vbe_mode_info->height-lines*font_height;i++)
        memmove((void*)vbe_mode_info->framebuffer+i*vbe_mode_info->pitch,
    (const void*)vbe_mode_info->framebuffer+(lines*font_height+i)*vbe_mode_info->pitch,
vbe_mode_info->pitch);
    memset((void*)vbe_mode_info->framebuffer+(vbe_mode_info->height-lines*font_height)*vbe_mode_info->pitch,
'\0', font_height*vbe_mode_info->pitch);
}