#pragma once
#include "../../hal/interrupt/irq.h"

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

struct ahci_ports_t;
typedef struct ahci_ports_t ahci_ports_t;

void ahci_interrupt_setup(hba_memory_t* abar, ahci_ports_t* port_container, u8 irq);