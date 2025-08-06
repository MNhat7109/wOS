#pragma once
#include "../../stdint.h"
#include <stdbool.h>

struct acpi_driver_t;
struct cpu_driver_t;

bool cpu_scan_madt(void (*callback)(void*));
bool cpu_scan_mpt(void (*callback)(void*));
bool cpu_prepare_apic_acpi();
bool cpu_prepare_apic_msr(struct cpu_driver_t* driver);

u64 cpu_timer_get_freq(u64 elapsed, u64 timer_freq);
void cpu_timer_init(u32 ticks, u8 vector);
u32 cpu_irq_to_gsi(u8 irq);
void cpu_redirect_gsi(u8 irq, u8 vector, u8 id);
void cpu_cut_gsi(u8 irq);
void cpu_timer_countdown();
u32 cpu_get_core_id();
void cpu_send_eoi(u8 _reserved_for_arm64);