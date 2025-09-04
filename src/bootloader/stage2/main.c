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
#include "rsdp/rsdp.h"

typedef struct
{
    u32 entries_count;
    struct
    {
        u64 base, length;
        u32 type;
        u32 acpi;
    } __attribute__((packed)) regions[];
} __attribute__((packed)) memory_info_t;

struct boot_info_t
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    system_desc_ptr_t* sdp;
    font_t* font_out;
    memory_info_t* mem_map;
} __attribute__((packed)) boot_info;

typedef void (*kernel_func_t)(struct boot_info_t* boot_info);

memory_info_t* mmap;

void __attribute__((cdecl)) start(u16 bootDrive, void* partitionOffset, memory_info_t* memInfo)
{
    // Save params,
    // just in case of a fucking tragedy where I "accidentally" insert the wrong value to disk.
    void* offset = partitionOffset;
    mmap = memInfo;
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
    // HACK: Reserve low memory, no reason for the kernel to still use it
    memInfo->regions[0].type = 2;
    
    for (int i=0;i<memInfo->entries_count;i++)
    {
        kprintf("Base: 0x%llx, Length: 0x%llx, Type: %u\n", memInfo->regions[i].base
        ,memInfo->regions[i].length, memInfo->regions[i].type);
    }
    kprintf("Loading the kernel...\n");
    kernel_func_t kernel_init;
    if (!ELF_load_file("/kernel.elf", (void**)&kernel_init))
    {
        kprintf("Failed to load kernel\n");
        goto end;
    }
    
    if (!RSDP_scan(&boot_info.sdp))
    {
        kprintf("Could not find RSDP!\n");
    }
    
    kprintf("\n"); //EOF
    boot_info.partition_offset = offset;
    boot_info.framebuffer = &fb_out;
    boot_info.font_out = &font;
    boot_info.mem_map = mmap;
    save_cursor();
    // PIC_disable();
    kprintf("BOOT: 0x%x\n", &boot_info);
    kernel_init(&boot_info);
end:    
    for (;;);
}