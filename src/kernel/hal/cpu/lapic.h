#pragma once
#include "apic.h"
#include "../../stdint.h"
#include <stdbool.h>

#define LAPIC_REG_LVT 0x320
#define LAPIC_REG_INITCNT 0x380
#define LAPIC_REG_CURRCNT 0x390
#define LAPIC_REG_DIVCFG 0x3E0

#define LAPIC_TIMER_MODE_ONE_SHOT 0x0
#define LAPIC_TIMER_MODE_PERIODIC 0x1
#define LAPIC_TIMER_MODE_TSC_DLINE 0x2

#define LAPIC_TIMER_DIVIDE_2 0x0
#define LAPIC_TIMER_DIVIDE_4 0x1
#define LAPIC_TIMER_DIVIDE_8 0x2
#define LAPIC_TIMER_DIVIDE_16 0x3
#define LAPIC_TIMER_DIVIDE_32 0x4
#define LAPIC_TIMER_DIVIDE_64 0x5
#define LAPIC_TIMER_DIVIDE_128 0x6
#define LAPIC_TIMER_DIVIDE_1 0x7

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
void LAPIC_timer_init(u8 vector, u32 tick_count, u8 timer_mode, u8 divide_mode);
u32 LAPIC_get_id();
void LAPIC_send_eoi();
// TODO: void LAPIC_cpu_init();
void LAPIC_write(u16 offset, u32 value);
u32 LAPIC_read(u16 offset);