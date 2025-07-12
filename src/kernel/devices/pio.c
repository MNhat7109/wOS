#include "pio.h"
#include "../x86/x86.h"

u8 pio_read_8(u16 port)
{
    return _x86_inb(port);
}

u16 pio_read_16(u16 port)
{
    return _x86_inw(port);
}

u32 pio_read_32(u16 port)
{
    return _x86_inl(port);
}

void pio_write_8(u16 port, u8 value)
{
    _x86_outb(port, value);
}

void pio_write_16(u16 port, u16 value)
{
    _x86_outw(port,value);
}

void pio_write_32(u16 port, u32 value)
{
    _x86_outl(port,value);
}

pio_layer_t pio_load_defaults()
{
    pio_layer_t layer = {
        .readb=pio_read_8,
        .readw=pio_read_16,
        .readl=pio_read_32,
        .writeb=pio_write_8,
        .writew=pio_write_16,
        .writel=pio_write_32
    };
    return layer;
}