#include "hal.h"
#include "gdt/gdt.h"
#include "interrupt/idt.h"
#include "interrupt/isr.h"
#include "interrupt/irq.h"
#include "cpu/apic.h"
#include "../stdio.h"

void HAL_init()
{
    GDT_init();
    IDT_init();
    ISR_init();
}

void HAL_init_stage2()
{
    if (!APIC_init())
    {
        kprintf("HAL: Cannot set up APIC. No interrupts will be raised as a result.\n");
        return;
    }
    IRQ_init();
}