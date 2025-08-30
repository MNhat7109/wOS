#include <arch/x86/utils/boot.h>
#include <arch/x86/utils/kernel/kernel.h>

void kstart(boot_info_t* boot_inf)
{
    // First steps
    kernel_prepare(boot_inf);

    // Init hardware interrupts

    // Init ktime

    // Disk setup

    // Scheduling setup
    
end:    for (;;);
}