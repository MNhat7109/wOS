#include "irq.h"
#include "../../stdint.h"
#include "../../stdio.h"
#include "../../x86/x86.h"
#include "../cpu/ioapic.h"
#include "../cpu/lapic.h"

gsi_handler_t gsi_handler_table[MAX_HANDLED_GSI];

void IRQ_end()
{
    LAPIC_send_eoi();
}

void irq_handler(registers_t* regs)
{
    int gsi = regs->vector - APIC_REMAP_OFFSET;

    if (gsi_handler_table[gsi]) gsi_handler_table[gsi](regs);
    else
    {
        kprintf("PIC: GSI no. %d unhandled!", gsi);
    }

    IRQ_end();
}

void IRQ_init()
{
    for (int i=0;i<MAX_HANDLED_GSI;i++)
    {
        ISR_reg_handler(APIC_REMAP_OFFSET+i, irq_handler);
    }
    _x86_enable_interrupt();
}

void IRQ_reg_handler(int gsi, gsi_handler_t handler)
{
    gsi_handler_table[gsi] = handler;
}

void IRQ_setup(int gsi_num, gsi_handler_t handler)
{
    if (gsi_num < 0 || gsi_num >= MAX_HANDLED_GSI) return;

    IRQ_reg_handler(gsi_num, handler);
    IOAPIC_redirect_gsi(gsi_num, APIC_REMAP_OFFSET+gsi_num, LAPIC_get_id());
}