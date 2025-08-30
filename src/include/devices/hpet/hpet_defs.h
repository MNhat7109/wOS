#pragma once
#include <libk/stdint.h>
#include <devices/acpi/acpi_defs.h>

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u8 revision_id;
    u8 comparator_count : 5;
    u8 counter_size : 1;
    u8 _reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendor_id;
    struct hpet_address_t
    {
        u8 address_space_id;
        u8 register_bit_width;
        u8 register_bit_offset;
        u8 _reserved;
        u64 base;
    } __attribute__((packed)) hpet_address;
    u8 hpet_num;
    u16 min_tick;
    u8 page_protection;
} __attribute__((packed)) hpet_t;

struct acpi_driver_t;
extern struct hpet_shared_t
{
    struct acpi_driver_t* acpi_dev;
    hpet_t* hpet_table;
    u32 hpet_phys_base;
} hpet;