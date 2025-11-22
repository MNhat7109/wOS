#include "irq.h"
#include "pic.h"
#include "../stdint.h"
#include "../stdio.h"
#include "../x86/x86.h"

#define PIC_REMAP_OFFSET 0x20

irq_handler_t* irq_handler_table = (irq_handler_t*)0x10C00;

void irq_handler(registers_t* regs)
{
    int irq = regs->vector - PIC_REMAP_OFFSET;

    u8 isr = PIC_read_isr(); u8 irr = PIC_read_irr();
    if (irq_handler_table[irq]) irq_handler_table[irq](regs);

    PIC_send_eoi(irq);
}

void IRQ_init()
{
    PIC_config(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET+8);
    for (int i=0;i<16;i++)
    {
        ISR_reg_handler(PIC_REMAP_OFFSET+i, irq_handler);
    }
    _x86_enable_interrupt();
}

void IRQ_reg_handler(int irq, irq_handler_t handler)
{
    irq_handler_table[irq] = handler;
}