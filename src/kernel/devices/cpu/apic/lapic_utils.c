#include "lapic.h"
#include "lapic_utils.h"
#include "lapic_defs.h"

void lapic_write(struct lapic_driver_t* self, u32 offset, u32 value)
{
    self->mmio.writel(self->__lapic_base, offset, value);
}

u32 lapic_read(struct lapic_driver_t* self, u32 offset)
{
    return self->mmio.readl(self->__lapic_base, offset);
}

u8 lapic_cook_div_mode(u8 mode)
{
    u8 div_mode_lo = mode & 0b11;
    u8 div_mode_hi = (mode>>2) & 0b11;
    div_mode_hi <<= 1;
    return div_mode_lo | (div_mode_hi<<2);
}
