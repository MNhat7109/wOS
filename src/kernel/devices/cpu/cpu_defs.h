#pragma once
#include "../../stdint.h"

struct lapic_driver_t;
struct ioapic_driver_t;
struct acpi_driver_t;
struct madt_t;
typedef struct madt_t madt_t;

extern struct x86_cpu_shared_t
{
    struct lapic_driver_t* lapic_layer;
    struct ioapic_driver_t* ioapic_layer;
    struct acpi_driver_t* acpi_driver;
    u8 msr_support, cpuid_support, acpi_support;
    madt_t* madt;
    u32 rec_len;
} x86_generic_cpu;