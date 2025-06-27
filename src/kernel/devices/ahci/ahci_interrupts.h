#pragma once
#include "../../hal/interrupt/irq.h"

typedef void (*ahci_handler_t)(ahci_port_t* port);

void ahci_register_handler(int inum, ahci_handler_t handler);
void ahci_interrupt_handler(registers_t* regs);

void ahci_handle_tfes(ahci_port_t* port);