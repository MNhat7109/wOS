#pragma once
#include "../../driver.h"
#include "../../mmio.h"

struct cpu_driver_t;
struct lapic_driver_t
{
    struct generic_driver_t driver_hdr;
    struct cpu_driver_t* cpu_dev;
    mmio_layer_t mmio;
    u32 __lapic_base;
    void (*write)(struct lapic_driver_t*, u32, u32);
    u32 (*read)(struct lapic_driver_t*, u32);
    u8 (*get_div_value)(u8);
} __attribute__((packed));

const struct lapic_driver_t* lapic_get_driver();