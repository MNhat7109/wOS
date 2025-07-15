#include "stdint.h"
#include "stdio.h"
#include "string/string.h"
#include "boot.h"
#include "paging/paging.h"
#include "hal/hal.h"
#include "acpi/acpi.h"
#include "pci/pci.h"
#include "disk/disk.h"
#include "ktime/ktime.h"
#include "scheduling/scheduling.h"

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));

    // Set up display
    video_init(boot_inf->framebuffer, boot_inf->font_out);
    kprintf("Hello from kernel!\n");
    
    // Set up ACPI
    ACPI_init(boot_inf->sdp);
    // Set up GDT, IDT, ISRs, IRQs
    HAL_init();
    
    // Set up memory
    memory_init(boot_inf->mem_map);
    memory_view_map();
    paging_init(boot_inf);
    u32 heap_page_count = 1024;
    memory_init_alloc(0xa000000, heap_page_count);
    
    // Set up PCI
    mcfg_t* mcfg = (mcfg_t*)ACPI_find_table("MCFG");
    PCI_init(mcfg);
    
    // Set up HAL stage 2
    HAL_init_stage2();
    
    // Set up timer for sleep()
    ktime_init();
    // Set up scheduling for multitasking
    scheduling_init();

    // Set up storage
    disk_init();

end:    for (;;);
}