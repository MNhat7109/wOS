#include <devices/pio.h>
#include <arch/x86/common/io.h>

static u8 pio_read_8(u16 port);
static u16 pio_read_16(u16 port);
static u32 pio_read_32(u16 port);
static void pio_write_8(u16 port, u8 value);
static void pio_write_16(u16 port, u16 value);
static void pio_write_32(u16 port, u32 value);

pio_layer_t pio_layer = {
    .readb=&pio_read_8,
    .readw=&pio_read_16,
    .readl=&pio_read_32,
    .writeb=&pio_write_8,
    .writew=&pio_write_16,
    .writel=&pio_write_32
};

int pio_acquire(struct pio_info_t* self, u16 port)
{
    self->port = port;
    self->layer = &pio_layer;
    return 0;
}

int pio_release(struct pio_info_t* self)
{
    self->port = 0;
    self->layer = NULL;
    return 0;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

static u8 pio_read_8(u16 port)
{
    return _x86_inb(port);
}

static u16 pio_read_16(u16 port)
{
    return _x86_inw(port);
}

static u32 pio_read_32(u16 port)
{
    return _x86_inl(port);
}

static void pio_write_8(u16 port, u8 value)
{
    _x86_outb(port, value);
}

static void pio_write_16(u16 port, u16 value)
{
    _x86_outw(port,value);
}

static void pio_write_32(u16 port, u32 value)
{
    _x86_outl(port,value);
}