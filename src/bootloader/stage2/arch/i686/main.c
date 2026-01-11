#include "../../stdint.h"

#include "../../boot_info.h"
#include "../../drivers/console.h"
#include "../../drivers/timer.h"
#include "../../drivers/rsdp.h"
#include "../../drivers/interrupt.h"

#include "../../fs/disk.h"
#include "../../fs/partition.h"
#include "../../fs/fs.h"
#include "../../fs/fat.h"
#include "../../exec/elf.h"

#include "../../stdio.h"

#define MODULE_MAIN "MAIN"

boot_info_t boot_info;
typedef void (*kernel_func_t)(boot_info_t* boot_info);

void __attribute__((cdecl)) start(u16 bootDrive, 
    void* mbr_buffer, 
    framebuffer_t* framebuffer, 
    font_t* font, 
    memory_info_t* memInfo
)
{
    int status;
    boot_info = (boot_info_t){
        .framebuffer = framebuffer,
        .font_out = font,
        .mem_map = memInfo,
        // TODO: ACPI
    };
    
    console_init(CONSOLE_MODE_BOTH, &boot_info);

    kprintf("Test kprintf\n"); 
    kdebugf(DEBUG_INFO, "MAIN", "Test kdebugf 1 of 3: Info display\n");   
    kdebugf(DEBUG_WARN, "MAIN", "Test kdebugf 2 of 3: Warning display\n");   
    kdebugf(DEBUG_CRITICAL, "MAIN", "Test kdebugf 3 of 3: Critical error display\n");   

    interrupt_init();
    timer_init();
    rsdp_scan(&boot_info.sdp);
    
    status = disk_init(); if (status < 0) goto end;
    int disk_number = disk_find_boot_dev(mbr_buffer);
    if (disk_number < 0 ) goto end;   

    disk_t* disk; partition_t* part;
    status = disk_get(disk_number, &disk);
    if (status < 0) goto end;
    status = partition_get(disk, 0, &part);
    if (status < 0) goto end;

    fs_t fs;
    fs_init("fat32", fat_load_ops, &fs);
    status = fs_mount(&fs, disk, part);
    if (status < 0) goto end;

    kernel_func_t kernel_init;
    status = elf_load_32(&fs, "kernel.elf", (void**)&kernel_init);
    if (status < 0) goto end;

    kdebugf(DEBUG_INFO, MODULE_MAIN, "BOOT_OK\n");
end:    
    for (;;);
}