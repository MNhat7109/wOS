#include <boot/boot_info.h>
#include <kernel/video.h>
#include <stdbool.h>

typedef struct boot_info_t boot_info_t;

u32 current_fg, current_bg;

void video_default_clear(u32 color);
void video_default_putchar(char ch, u32 x, u32 y, u32 fg, u32 bg);
void video_default_putpix(u32 x, u32 y, u32 color);
void video_default_scroll(u32 lines);
void video_default_set_cursor(u32 x, u32 y);

video_ops_t video_default_ops = {
    .clear = &video_default_clear,
    .putchar = &video_default_putchar,
    .putpix = &video_default_putpix,
    .scroll = &video_default_scroll,
    .set_cursor = &video_default_set_cursor
};

video_ops_t* video_ops = &video_ops;

int lfb_init(framebuffer_t* fb, font_t* font);
void video_fallback_init();

void video_init(boot_info_t* boot_info)
{
    if (!boot_info) goto fallback;
    int status = lfb_init(boot_info->framebuffer, boot_info->font_out);
    if (status < 0) goto fallback;
    return;
fallback:
    video_fallback_init();
}

void video_default_clear(u32 color){}
void video_default_putchar(char ch, u32 x, u32 y, u32 fg, u32 bg){}
void video_default_putpix(u32 x, u32 y, u32 color){}
void video_default_scroll(u32 lines){}
void video_default_set_cursor(u32 x, u32 y){}