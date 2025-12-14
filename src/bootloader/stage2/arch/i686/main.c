#include "../../stdint.h"

#include "../../boot_info.h"
#include "../../drivers/console.h"
#include "../../drivers/timer.h"
#include "../../drivers/rsdp.h"
#include "../../drivers/disk.h"
#include "../../drivers/interrupt.h"
#include "../../stdio.h"

boot_info_t boot_info;
typedef void (*kernel_func_t)(boot_info_t* boot_info);

void __attribute__((cdecl)) start(u16 bootDrive, 
    void* partitionOffset, 
    framebuffer_t* framebuffer, 
    font_t* font, 
    memory_info_t* memInfo
)
{
    boot_info = (boot_info_t){
        .framebuffer = framebuffer,
        .font_out = font,
        .mem_map = memInfo,
        .partition_offset = partitionOffset,
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
    disk_init();

end:    
    for (;;);
}