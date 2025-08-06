#include "stdint.h"
#include "stdio.h"
#include "boot.h"
#include "peripherals.h"
#include "hal/hal.h"
#include "disk/disk.h"
#include "time.h"
#include "ktime/ktime.h"
#include "scheduling/scheduling.h"

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    if (!boot_prepare(boot_inf))
    {
        goto end;
    }

    boot_prepare_acpi();

    peripherals_init();

    HAL_init_essentials();
    
    // Set up timer for sleep()
    ktime_init();
    
    // Set up storage
    disk_init();
    
    // Set up scheduling for multitasking
    // scheduling_init();
    
end:    for (;;);
}