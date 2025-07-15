#pragma once
#include "../../acpi/acpi.h"
#include "../../stdint.h"
#include <stdbool.h>

typedef struct
{
    u8 entry_type;
    u8 record_length;
} __attribute__((packed)) madt_record_entry_hdr_t;

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 nmi_source;
    u16 flags;
    u32 gsi;
} __attribute__((packed)) madt_ioapic_nmi_src_t;

typedef struct
{
    acpi_sdt_hdr_t table_hdr;
    u32 lapic_addr;
    u32 flags;
    madt_record_entry_hdr_t records[1];
} __attribute__((packed)) madt_t;

typedef void (*madt_callback_t)(madt_record_entry_hdr_t* ptr);

void APIC_scan_hdr(madt_callback_t callback);
bool APIC_init();