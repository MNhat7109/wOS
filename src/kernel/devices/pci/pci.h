#pragma once
#include "../driver.h"
#include <stdbool.h>

struct pci_driver_t
{
    struct generic_driver_t driver_hdr;
    u32 ecam_base;
    u8 bus,dev,func;
    void (*write)(struct pci_driver_t*, u8, u32);
    u32 (*read)(struct pci_driver_t*, u8);
    bool (*scan)(struct pci_driver_t*, bool (*)(struct pci_driver_t*));
} __attribute__((packed));

const struct generic_driver_t* pci_get_driver();