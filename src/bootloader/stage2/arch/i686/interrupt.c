#include "../../drivers/interrupt.h"
#include "../../stdio.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

void interrupt_init()
{
    i686_idt_init();
    i686_isr_init();
    i686_irq_init();
    kdebugf(DEBUG_INFO, "INTERRUPT", "Interrupt is set up.\n");
}