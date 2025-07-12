#include "mmio.h"

u32 mmio_read_32(u32 base, u32 offset)
{
    return *((volatile u32*)(base+offset));
}

u64 mmio_read_64(u32 base, u32 offset)
{
    return *((volatile u64*)(base+offset));
}

void mmio_write_32(u32 base, u32 offset, u32 value)
{
    *((volatile u32*)(base+offset)) = value;
}

void mmio_write_64(u32 base, u32 offset, u64 value)
{
    *((volatile u64*)(base+offset)) = value;
}

mmio_layer_t mmio_load_defaults()
{
    mmio_layer_t layer = {
        .readl = &mmio_read_32,
        .readq = &mmio_read_64,
        .writel = &mmio_write_32,
        .writeq = &mmio_write_64
    };
    return layer;
}