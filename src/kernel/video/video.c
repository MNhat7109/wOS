#include <boot/boot_info.h>
#include <kernel/video.h>
#include <kernel/console.h>
#include <stdbool.h>

typedef struct boot_info_t boot_info_t;
typedef struct framebuffer_t framebuffer_t;

console_backend_t* video_backend;
u32 video_cols=0, video_rows=0, video_fg, video_bg;

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

console_backend_t* video_get_backend()
{
    return video_backend;
}