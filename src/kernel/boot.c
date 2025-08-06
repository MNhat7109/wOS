#include "boot.h"

#include "devices/acpi/acpi.h"
#include "devices/ahci/ahci.h"
#include "devices/cpu/cpu.h"
#include "devices/i8259/i8259.h"
#include "devices/i8254/i8254.h"
#include "devices/hpet/hpet.h"
#include "devices/pci/pci.h"
#include "devices/driver.h"

#include "string/string.h"
#include "stdio.h"
#include "video/video.h"
#include "memory/memory.h"
#include "paging/paging.h"
#include "hal/hal.h"

boot_info_t* bootloader_info;
struct generic_driver_t* acpi_device; 

bool boot_prepare(boot_info_t* info)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));
    bootloader_info = info;
    
    // Set up display
    video_init(bootloader_info->framebuffer, bootloader_info->font_out);

    // Set up GDT, IDT, ISRs
    HAL_init_boot();
    
    // Set up memory
    memory_init(bootloader_info->mem_map);
    memory_view_map();
    paging_init(bootloader_info);
    u32 heap_page_count = 1024;
    memory_init_alloc(0xa000000, heap_page_count);
    // TODO: We desperately need a memory map for this, can't just
    // sprinkle magic numbers like this one (1024, ew)

    // Load all the drivers first

    driver_load(acpi_get_driver);

    // Interrupt routing
    driver_load(cpu_get_driver);
    driver_load(i8259_get_driver);

    // Peripherals
    driver_load(pci_get_driver);

    // Timer
    driver_load(hpet_get_driver);
    driver_load(i8254_get_driver);

    // Disk
    driver_load(ahci_get_driver);

    return true;
}

void boot_prepare_acpi()
{
    // Set up ACPI
    acpi_device = driver_get("ACPI");
    if (!acpi_device)
    {
        kprintf("Kernel: ACPI driver not found\n");
    }

    if (!driver_run(acpi_device))
    {
        kprintf("Kernel: Failed to start ACPI driver\n");
        driver_unload("ACPI");
    }
}