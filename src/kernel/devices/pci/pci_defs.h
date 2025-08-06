#pragma once
#include "../acpi/acpi_defs.h"
#include "../mmio.h"
#include "../pio.h"

#define PCI_ACPI_PRESENT (1<<0)
#define PCI_MCFG_PRESENT (1<<1)

#define PCI_HDR_TYPE_MULTIFUNC (1<<7)

typedef struct
{
    u64 base;
    u16 group_number;
    u8 bus_start;
    u8 bus_end;
    u32 reserved;
} __attribute__((packed)) pci_dev_cfg_t;

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u64 reserved;
} __attribute__((packed)) mcfg_t;

struct acpi_driver_t;
extern struct pci_shared_t
{
    struct acpi_driver_t* acpi_dev;
    mcfg_t* mcfg;
    u32 mcfg_entries;
    u8 mmio_available;
    pio_layer_t  pio_utils;
    mmio_layer_t mmio_utils;
} pci;