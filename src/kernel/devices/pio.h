#pragma once
#include "../stdint.h"

typedef struct
{
    u8 (*readb)(u16 port);
    u16 (*readw)(u16 port);
    u32 (*readl)(u16 port);
    void (*writeb)(u16 port, u8 value);
    void (*writew)(u16 port, u16 value);
    void (*writel)(u16 port, u32 value);
} pio_layer_t;

pio_layer_t pio_load_defaults();