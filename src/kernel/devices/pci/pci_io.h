#pragma once
#include <stdbool.h>
#include "../../stdint.h"

struct pci_driver_t;
bool pci_io_scan(struct pci_driver_t* self, bool (*callback)(struct pci_driver_t*));
u32 pci_io_read(struct pci_driver_t* self, u8 offset);
void pci_io_write(struct pci_driver_t* self, u8 offset, u32 value);