#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    u16 device_id;
    u16 vendor_id;
    u16 type;
    u8 bus, slot, func;
} pci_dev_t;

typedef bool (*pci_dev_func_t)(pci_dev_t* dev);

u16 PCI_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);
bool PCI_scan(pci_dev_func_t callback);
u8 PCI_get_irq_vector(pci_dev_t* dev);