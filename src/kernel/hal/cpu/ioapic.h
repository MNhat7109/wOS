#pragma once
#include "../../stdint.h"
#include "apic.h"
#include <stdbool.h>

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


bool IOAPIC_init();
void IOAPIC_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id);
u32 IOAPIC_irq_to_gsi(u8 irq);
u32 IOAPIC_get_max_gsi();