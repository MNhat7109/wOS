#include "stdint.h"
#include "video/video.h"

typedef struct 
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    font_t* font_out;
} boot_info_t;

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    for (;;);
}