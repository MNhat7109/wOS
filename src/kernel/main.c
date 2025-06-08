#include "stdint.h"
#include "stdio.h"
#include "string/string.h"
#include "boot.h"
#include "paging/paging.h"
#include "hal/hal.h"
#include "acpi/acpi.h"
#include "pci/pci.h"
#include "scheduling/timer.h"
#include "scheduling/tss.h"
#include "scheduling/multitasking.h"

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));

    // Set up display
    video_init(boot_inf->framebuffer, boot_inf->font_out);
    kprintf("Hello from kernel!\n");
    
    // Set up GDT, IDT, ISRs, IRQs
    HAL_init();
    
    // Set up memory
    memory_init(boot_inf->mem_map);
    memory_view_map();
    paging_init(boot_inf);
    u32 heap_page_count = page_convert_from_bytes(page_get_free_mem()/4);
    memory_init_alloc(0x500000, heap_page_count);

    // Set up ACPI
    ACPI_init(boot_inf->sdp);

    // Set up PCI
    mcfg_t* mcfg = (mcfg_t*)ACPI_find_table("MCFG");
    PCI_init(mcfg);

    // Set up storage
    
    // Set up scheduling for multitasking
    timer_init();
    TSS_init();

end:    for (;;);
}