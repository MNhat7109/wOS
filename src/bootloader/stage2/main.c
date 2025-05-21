#include "stdint.h"
#include "stdio.h"
#include "interrupt/interrupt.h"
#include "interrupt/pic.h"
#include "timer/pit.h"
#include "disk/disk.h"
#include "fat/fat.h"
#include "elf/elf.h"
#include "vesa/vesa.h"
#include "vesa/font.h"

struct boot_info_t
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    font_t* font_out;
} boot_info;

typedef void (*kernel_func_t)(struct boot_info_t* boot_info);

void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset)
{
    // Save params,
    // just in case of a fucking tragedy where I "accidentally" insert the wrong value to disk.
    void* offset = partitionOffset;
    interrupt_init();
    PIT_init(2000);
    kprintf("Hello from stage2!\n");
    // Welp, I guess we can write a FAT32 filesystem driver now, eh?
    if (!FAT_init(0, offset))
    {
        kprintf("FAT init failed\n");
        goto end;
    }
    
    if (!font_init("fonts/font.psf"))
    {
        kprintf("Font init failed\n");
        goto end;
    }
    VESA_init();
    kprintf("Loading the kernel...\n");
    kernel_func_t kernel_init;
    if (!ELF_load_file("/kernel.elf", (void**)&kernel_init))
    {
        kprintf("Failed to load kernel\n");
        goto end;
    }
    kprintf("\n"); //EOF
    boot_info.partition_offset = offset;
    boot_info.framebuffer = &fb_out;
    boot_info.font_out = &font;
    save_cursor();
    // PIC_disable();
    kernel_init(&boot_info);
end:    
    for (;;);
}