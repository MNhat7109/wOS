#pragma once
#include <libk/stdint.h>

#define PIO_ERR_UNSUPPORTED 1

typedef struct pio_layer_t
{
    u8 (*readb)(u16 port);
    u16 (*readw)(u16 port);
    u32 (*readl)(u16 port);
    void (*writeb)(u16 port, u8 value);
    void (*writew)(u16 port, u16 value);
    void (*writel)(u16 port, u32 value);
} pio_layer_t;

struct pio_info_t
{
    u16 port;
    pio_layer_t* layer;
};

int pio_acquire(struct pio_info_t* self, u16 port);
int pio_release(struct pio_info_t* self);
pio_layer_t pio_load_defaults();