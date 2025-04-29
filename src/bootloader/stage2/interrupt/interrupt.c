#include "interrupt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "../stdio.h"

static int a = 0;
void timer(registers_t* regs)
{
    a++;
    kprintf("%d\n", a);
    return;
}

void interrupt_init()
{
    IDT_init();
    ISR_init();
    IRQ_init();
    //IRQ_reg_handler(0, timer);
}