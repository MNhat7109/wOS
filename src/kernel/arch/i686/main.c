#include <stdint.h>

typedef struct boot_info_t boot_info_t;

void kstart(boot_info_t* boot_inf)
{
    // if (!boot_prepare(boot_inf))
    // {
    //     goto end;
    // }

    // boot_prepare_acpi();

    // peripherals_init();

    // HAL_init_essentials();
    
    // // Set up timer for sleep()
    // ktime_init();
    
    // // Set up storage
    // disk_init();
    
    // // Set up scheduling for multitasking
    // // scheduling_init();
    
end:    for (;;);
}