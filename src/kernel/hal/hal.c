#include "hal.h"
#include "gdt/gdt.h"
#include "interrupt/idt.h"
#include "interrupt/isr.h"
#include "interrupt/irq.h"

void HAL_init()
{
    GDT_init();
    IDT_init();
    ISR_init();
    IRQ_init();
    // __asm__ volatile ("sti");
}