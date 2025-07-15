#pragma once
#include "../stdint.h"

typedef struct
{
    u32 (*readl)(u32 base, u32 offset);
    u64 (*readq)(u32 base, u32 offset);
    void (*writel)(u32 base, u32 offset, u32 value);
    void (*writeq)(u32 base, u32 offset, u64 value);
} mmio_layer_t;

mmio_layer_t mmio_load_defaults();
