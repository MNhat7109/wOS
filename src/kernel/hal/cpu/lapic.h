#pragma once
#include "apic.h"
#include "../../stdint.h"
#include <stdbool.h>

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 acpi_proc_id;
    u8 apic_id;
    u32 flags;
} __attribute__((packed)) madt_record_lapic_t;

// TODO
// typedef struct
// {

// } __attribute__((packed)) cpu_info_t;

#define LAPIC_SVR_APIC (1<<8)

bool LAPIC_init(u32 lapic_base);
void LAPIC_timer_init(u8 vector, u32 tick_count);
u32 LAPIC_get_id();
void LAPIC_send_eoi();
// TODO: void LAPIC_cpu_init();
void LAPIC_write(u16 offset, u32 value);
u32 LAPIC_read(u16 offset);