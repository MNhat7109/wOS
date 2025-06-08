#pragma once
#include "../acpi/acpi.h"
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u64 reserved;
} __attribute__((packed)) mcfg_t;

typedef struct
{
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8 revision_id;
    u8 prog_if;
    u8 sub;
    u8 class;
    u8 cache_line_size;
    u8 latency_timer;
    u8 hdr_type;
    u8 bist;
} __attribute__((packed)) pci_dev_hdr_t;

typedef struct
{
    u16 vendor_id;
    u16 device_id;
    struct
    {
        u8 sub, class;
    } __attribute__((packed)) type;
    u8 bus, slot, func;
} __attribute__((packed)) pci_dev_t;

typedef bool (*pci_dev_func_t)(pci_dev_t* dev);

void PCI_init(mcfg_t* mcfg_table);

bool PCI_scan(pci_dev_func_t callback);