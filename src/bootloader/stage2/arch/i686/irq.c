#include "irq.h"
#include "../../drivers/pic.h"
#include "../../stdint.h"

#define PIC_REMAP_OFFSET 0x20

irq_handler_t irq_handler_table[16];

void irq_handler(registers_t* regs)
{
    int irq = regs->vector - PIC_REMAP_OFFSET;

    u8 isr = PIC_read_isr(); u8 irr = PIC_read_irr();
    if (irq_handler_table[irq]) irq_handler_table[irq](regs);

    PIC_send_eoi(irq);
}

void i686_irq_init()
{
    PIC_config(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET+8);
    for (int i=0;i<16;i++)
    {
        i686_isr_register_handler(PIC_REMAP_OFFSET+i, irq_handler);
    }
    i686_enable_interrupt();
}

void i686_irq_register_handler(int irq, irq_handler_t handler)
{
    irq_handler_table[irq] = handler;
}