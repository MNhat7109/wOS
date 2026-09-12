#include <kernel/sys/acpi.h>
#include <kernel/arch/i686/apic.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

// #define APIC_REMAP_OFFSET 0x30

static apic_tbl_t madt_container;

int lapic_init(apic_tbl_t* madt);
int ioapic_init(apic_tbl_t* madt);

int arch_sys_init()
{
    int st;

    st = apic_set_up_madt(&madt_container);
    if (st < 0) return st;
    
    st = lapic_init(&madt_container);
    if (st < 0) return st;

    st = ioapic_init(&madt_container);
    if (st < 0) return st;
    return 0;
}

int arch_sys_init_fallback()
{
    int st;

    st = pic_init();
    if (st < 0) return st;

    st = pit_init();
    if (st < 0) return st;
}

