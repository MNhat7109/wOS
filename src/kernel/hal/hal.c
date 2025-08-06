#include "hal.h"
#include "gdt/gdt.h"
#include "interrupt/idt.h"
#include "interrupt/isr.h"
#include "interrupt/irq.h"

void HAL_init_boot()
{
    GDT_init();
    IDT_init();
    ISR_init();
}

void HAL_init_essentials()
{
    IRQ_init();
}