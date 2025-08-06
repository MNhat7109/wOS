#pragma once
#include "apic_defs.h"
#include "../../../stdint.h"
#include <stdbool.h>

#define MAX_IOAPIC_ENTRIES 256
#define MAX_ISO_ENTRIES 16

#define IOAPIC_REGSEL 0x00
#define IOAPIC_REGWIN 0x10

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 ioapic_id;
    u8 _reserved;
    u32 ioapic_addr;
    u32 gsi_base; // Global System Interrupt Base
} __attribute__((packed)) madt_record_ioapic_t;

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 bus_source;
    u8 irq_source;
    u32 gsi;
    u16 flags;
} __attribute__((packed)) madt_record_ioapic_iso_t;

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 nmi_source;
    u16 flags;
    u32 gsi;
} __attribute__((packed)) madt_ioapic_nmi_src_t;

extern struct ioapic_shared_t
{
    struct ioapic_info_t
    {
        u32 id;
        u32 max_redirs;
        u32 ioapic_base;
        u32 gsi_base;
    } __attribute__((packed)) *ioapic_list;

    struct iso_info_t
    {
        bool present;
        u8 legacy_irq_num;
        u16 flags;
        u32 gsi_num;
    } __attribute__((packed)) *iso_map;

    u32 current_ioapic_count;
    u32 max_gsi_count;

    struct ioapic_driver_t* driver;
} ioapic;