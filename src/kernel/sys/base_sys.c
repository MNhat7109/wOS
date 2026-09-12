#include <kernel/sys/base_sys.h>
#include <kernel/sys/hw_interrupt.h>
#include <errno.h>

int arch_sys_init();
int arch_sys_init_fallback();

int base_sys_init()
{
    hw_interrupt_init();
    
    // Init arch-specific base system
    int st = arch_sys_init();
    if (st == 0) return 0;
    // No APIC? Fallback to PIC+PIT
    st = arch_sys_init_fallback();
    if (st < 0) return -ENOTSUP;
    return 0;
}