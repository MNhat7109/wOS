#include "video.h"
#include "vesa.h"
#include "vga.h"

#define DEFAULT_FG_COLOR VGA_COLOR_GRAY
#define DEFAULT_BG_COLOR VGA_COLOR_BLACK

static struct
{
    u8 current_fg, current_bg;
} video_data;

u32 video_vga_to_24bpp_color_map[16] =
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

bool video_vesa_on = false;

void video_init(boot_info_t* boot_info)
{
    VESA_init(boot_info->framebuffer, boot_info->font_out);
    if (VESA_check_status()) 
    {
        video_vesa_on=true;
    }

    video_data.current_fg = DEFAULT_FG_COLOR;
    video_data.current_bg = DEFAULT_BG_COLOR;
}

void video_putch(char ch, u32 x, u32 y)
{
    if (video_vesa_on) 
    VESA_putch(
        x, 
        y, 
        ch, 
        video_vga_to_24bpp_color_map[video_data.current_fg], 
        video_vga_to_24bpp_color_map[video_data.current_bg]
    );
    else 
    VGA_putch(
        x, 
        y, 
        ch, 
        video_data.current_fg, 
        video_data.current_bg
    );
}

void video_setcolor(u8 fg, u8 bg)
{
    video_data.current_fg = fg;
    video_data.current_bg = bg;
}

void video_getcolor(u8* fg_out, u8* bg_out)
{
    *fg_out = video_data.current_fg;
    *bg_out = video_data.current_bg;
}

void video_scroll(u32 lines)
{
    if (video_vesa_on) VESA_scroll(lines);
    else VGA_scroll(lines);
}

void video_clrscr()
{
    if (video_vesa_on) VESA_clear(video_vga_to_24bpp_color_map[video_data.current_bg]);
    else VGA_clear(video_data.current_bg);
}

void video_setcursor(u32 x, u32 y)
{
    if (video_vesa_on) return;
    else VGA_set_cursor(x,y);
}

u32 video_get_SCH()
{
    if (video_vesa_on) return VESA_get_SCH();
    return VGA_get_SCH();
}

u32 video_get_SCW()
{
    if (video_vesa_on) return VESA_get_SCW();
    return VGA_get_SCW();
}