#include "ahci_irq.h"
#include "ahci_defs.h"

void ahci_interrupt_legacy_setup(ahci_controller_t* ctl, gsi_handler_t handler)
{
    IRQ_setup(ctl->interrupt_num, handler, ctl);
}

void ahci_interrupt_legacy_disable(ahci_controller_t* ctl)
{
    IRQ_disable(ctl->interrupt_num);
}

void ahci_interrupt_legacy_end(u32 int_num)
{
    IRQ_end(int_num);
}