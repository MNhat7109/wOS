#pragma once
#include "../../../stdint.h"
#include "../../acpi/acpi_defs.h"

typedef struct
{
    u8 entry_type;
    u8 record_length;
} __attribute__((packed)) madt_record_entry_hdr_t;

typedef struct madt_t
{
    acpi_sdt_hdr_t table_hdr;
    u32 lapic_addr;
    u32 flags;
    madt_record_entry_hdr_t records[1];
} __attribute__((packed)) madt_t;
