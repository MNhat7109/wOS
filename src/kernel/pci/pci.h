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
    pci_dev_hdr_t hdr;
    u32 bar0;
    u32 bar1;
    u32 bar2;
    u32 bar3;
    u32 bar4;
    u32 bar5;
    u32 card_bus_cis_ptr;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u32 expansion_rom_base;
    u8 capabilities_ptr;
    u8 _reserved0;
    u16 _reserved1;
    u32 _reserved2;
    u8 interrupt_line;
    u8 interrupt_pin;
    u8 min_grant;
    u8 max_latency;
} __attribute((packed)) pci_hdr0_t;

typedef struct
{
    u16 vendor_id;
    u16 device_id;
    struct
    {
        u8 sub, class;
    } __attribute__((packed)) type;
    u8 bus, slot, func;
    u8 prog_if;
    u32 mmio_phys_addr;
} __attribute__((packed)) pci_dev_t;

typedef bool (*pci_dev_func_t)(pci_dev_t* dev);

u16 PCI_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);

void PCI_init(mcfg_t* mcfg_table);

bool PCI_scan(pci_dev_func_t callback);