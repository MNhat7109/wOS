#pragma once
#include "../../hal/interrupt/irq.h"
#include "../../stdint.h"

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;

struct generic_driver_t;

void ahci_interrupt_legacy_setup(ahci_controller_t* ctl, gsi_handler_t handler);
void ahci_interrupt_legacy_disable(ahci_controller_t* ctl);
void ahci_interrupt_legacy_end(u32 int_num);