#pragma once
#include "../driver.h"
#include "../mmio.h"
#include "../../acpi/acpi.h"

typedef struct
{
    u8 address_space_id;
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 _reserved;
    u64 base;
} __attribute__((packed)) hpet_address_t;

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u8 revision_id;
    u8 comparator_count : 5;
    u8 counter_size : 1;
    u8 _reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendor_id;
    hpet_address_t hpet_address;
    u8 hpet_num;
    u16 min_tick;
    u8 page_protection;
} __attribute__((packed)) hpet_t;

struct hpet_driver_t
{
    struct generic_driver_t driver_hdr;
    mmio_layer_t mmio_utils;
    hpet_t* hpet_acpi_table;
} __attribute__((packed));

const struct generic_driver_t* hpet_get_driver();