#pragma once
#include "../driver.h"
#include "../mmio.h"

struct hpet_driver_t
{
    struct generic_driver_t driver_hdr;
    mmio_layer_t mmio_utils;
    void (*write)(struct hpet_driver_t*, u32, u64);
    u64 (*read)(struct hpet_driver_t*, u32);
    void (*write32)(struct hpet_driver_t*, u32, u32);
    u32 (*read32)(struct hpet_driver_t*, u32);
} __attribute__((packed));

const struct generic_driver_t* hpet_get_driver();