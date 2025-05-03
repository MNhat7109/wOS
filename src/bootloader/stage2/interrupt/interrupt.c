#include "interrupt.h"
#include "idt.h"
#include "isr.h"

void interrupt_init()
{
    IDT_init();
    ISR_init();
    IRQ_init();
}