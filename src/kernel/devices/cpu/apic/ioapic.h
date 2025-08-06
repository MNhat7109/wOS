#pragma once
#include "../../driver.h"
#include "../../mmio.h"

struct ioapic_driver_t
{
    struct generic_driver_t driver_hdr;
    struct cpu_driver_t* cpu_dev;
    mmio_layer_t mmio;
    void (*write)(struct ioapic_driver_t*, u32, u8, u32);
    u32 (*read)(struct ioapic_driver_t*, u32, u8);
    void (*redirect_gsi)(struct ioapic_driver_t*, u8, u8, u8);
    void (*cut_gsi)(struct ioapic_driver_t*, u8);
    u32 (*irq_to_gsi)(struct ioapic_driver_t*, u8);
} __attribute__((packed));

const struct ioapic_driver_t* ioapic_get_driver();