#pragma once
#include <stdbool.h>
#include "../stdint.h"

#define PCI_HDR_TYPE_MULTIFUNC (1<<7)

typedef struct pci_device_t
{
    u8 bus,dev,func;
} pci_device_t;

bool pci_scan(pci_device_t* device, bool (*callback)(pci_device_t*));
u32 pci_read(pci_device_t* device, u8 offset);
void pci_write(pci_device_t* device, u8 offset, u32 value);