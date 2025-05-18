#include "stdint.h"
#include "video/video.h"
#include "string/string.h"

typedef struct 
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    font_t* font_out;
} boot_info_t;

extern u8 __bss_start;
extern u8 __end;

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));
    for (;;);
}