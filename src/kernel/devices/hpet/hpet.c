#include "hpet.h"
#include "../../stdint.h"
#include "../../stdio.h"
#include <stdbool.h>
#include "../../paging/paging.h"

bool hpet_probe(struct generic_driver_t* driver)
{
    struct hpet_driver_t* hpet_self = (struct hpet_driver_t*)driver;
    hpet_self->hpet_acpi_table = (hpet_t*)ACPI_find_table("HPET");
    return hpet_self->hpet_acpi_table;
}

void hpet_read_capabilities(struct hpet_driver_t* hpet_self)
{
    u32 hpet_phys = hpet_self->hpet_acpi_table->hpet_address.base;
    u64 hpet_cap = hpet_self->mmio_utils.readq(hpet_phys, 0);
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

void hpet_config(struct generic_driver_t* driver)
{
    struct hpet_driver_t* hpet_self = (struct hpet_driver_t*)driver;
    u32 hpet_phys = hpet_self->hpet_acpi_table->hpet_address.base;
    kprintf("HPET: Base=0x%x\n", hpet_phys);
    page_manager_map_memory(hpet_phys, hpet_phys);
    kprintf("HPET: Reading capabilities\n");
    hpet_read_capabilities(hpet_self);

    hpet_self->mmio_utils.writeq(hpet_phys, 0xF0, 0); // Reset main counter
    u64 conf = hpet_self->mmio_utils.readq(hpet_phys, 0x10);
    conf|=1;
    hpet_self->mmio_utils.writeq(hpet_phys, 0x10, conf); // Enable HPET
}

void hpet_disable(struct generic_driver_t* driver)
{
    struct hpet_driver_t* hpet_self = (struct hpet_driver_t*)driver;
    u32 hpet_phys = hpet_self->hpet_acpi_table->hpet_address.base;
    u64 conf = hpet_self->mmio_utils.readq(hpet_phys, 0x10);
    conf &= ~1;
    hpet_self->mmio_utils.writeq(hpet_phys, 0x10, conf);

    if (!page_manager_unmap_memory(hpet_phys))
    {
        kprintf("HPET: Could not unmap HPET MMIO address!\n");
        return;
    }
    // TODO: ACPI unmap table
}

struct hpet_driver_t hpet_driver = 
{ 
    .driver_hdr = {
        .name = "HPET",
        .config = &hpet_config,
        .probe = &hpet_probe,
        .disable = &hpet_disable
    },
    .hpet_acpi_table = NULL
};

const struct generic_driver_t* hpet_get_driver()
{
    hpet_driver.mmio_utils = mmio_load_defaults();
    return (struct generic_driver_t*)&hpet_driver;
}