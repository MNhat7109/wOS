#include "stdint.h"
#include "video/video.h"
#include "stdio.h"
#include "string/string.h"
#include "memory/memory.h"
#include "hal/hal.h"

typedef struct 
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    font_t* font_out;
    memory_info_t* mem_map;
} boot_info_t;

extern u8 __bss_start;
extern u8 __end;

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));
    video_init(boot_inf->framebuffer, boot_inf->font_out);
    kprintf("Hello from kernel!\n");
    // Set up GDT, IDT, ISRs, IRQs
    HAL_init();

    for (;;);
}