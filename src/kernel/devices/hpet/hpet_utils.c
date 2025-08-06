#include "hpet_utils.h"
#include "hpet.h"
#include "hpet_defs.h"
#include "../../stdio.h"

extern struct hpet_shared_t hpet;

void hpet_write(struct hpet_driver_t* self, u32 offset, u64 value)
{
    self->mmio_utils.writeq(hpet.hpet_phys_base, offset, value);
}

u64 hpet_read(struct hpet_driver_t* self, u32 offset)
{
    return self->mmio_utils.readq(hpet.hpet_phys_base, offset);
}

void hpet_write32(struct hpet_driver_t* self, u32 offset, u32 value)
{
    self->mmio_utils.writel(hpet.hpet_phys_base, offset, value);
}

u32 hpet_read32(struct hpet_driver_t* self, u32 offset)
{
    return self->mmio_utils.readl(hpet.hpet_phys_base, offset);
}

void hpet_read_capabilities(struct hpet_driver_t* self)
{
    u64 hpet_cap = self->read(self, 0);
    // TODO: Massive changes must be made in log_state()
    // before ditching kprintf() altogether
    kprintf("HPET Capability: Revision ID: %u\n"
        , (hpet_cap>>0) & 0xFF);
    kprintf("HPET Capability: Timer Count: %u\n"
        , ((hpet_cap>>8) & 0x1F)-1);
    kprintf("HPET Capability: 64-bit Mode Counter: %s\n"
        , (const char*[]){"no", "yes"}[(hpet_cap>>13) & 0x1]);
    kprintf("HPET Capability: Legacy Replacement Support: %s\n"
        , (const char*[]){"no", "yes"}[(hpet_cap>>15) & 0x1]);
    kprintf("HPET Capability: Vendor ID: %u\n"
        , (hpet_cap>>16) & 0xFFFF);
    kprintf("HPET Capability: Femtoseconds per Tick: %u\n"
        , (hpet_cap>>32) & 0xFFFFFFFF);
}