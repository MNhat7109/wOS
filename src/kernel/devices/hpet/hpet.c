#include "hpet.h"
#include "hpet_defs.h"
#include "hpet_utils.h"

#include "../acpi/acpi.h"
#include "../../paging/paging.h"

#include "../../stdint.h"
#include "../../stdio.h"
#include <stdbool.h>

struct hpet_shared_t hpet;

void hpet_probe(struct generic_driver_t* driver)
{
    struct hpet_driver_t* hpet_self = (struct hpet_driver_t*)driver;
    hpet.acpi_dev = (struct acpi_driver_t*)driver_get("ACPI");
    if (!driver_run((struct generic_driver_t*)hpet.acpi_dev))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Failed to start ACPI driver");
        return;
    }

    hpet.hpet_table = (hpet_t*)hpet.acpi_dev->get_table(hpet.acpi_dev, "HPET");
    if (!hpet.hpet_table)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "HPET table not found");
        return;
    }
}

void hpet_config(struct generic_driver_t* driver)
{
    struct hpet_driver_t* hpet_self = (struct hpet_driver_t*)driver;
   
    hpet.hpet_phys_base = hpet.hpet_table->hpet_address.base;
    kprintf("HPET: Base=0x%x\n", hpet.hpet_phys_base); 
    // TODO: Will update this after refining log_state() to include formatted strings
    page_manager_map_memory(hpet.hpet_phys_base, hpet.hpet_phys_base);

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Reading capabilities");
    hpet_read_capabilities(hpet_self);

    hpet_self->write(hpet_self, 0xF0, 0); // Reset main counter
    u64 conf = hpet_self->read(hpet_self, 0x10);
    conf|=1;
    hpet_self->write(hpet_self, 0x10, conf); // Enable HPET
}

void hpet_disable(struct generic_driver_t* driver)
{
    bool ok;
    ok = page_manager_unmap_memory(hpet.hpet_phys_base);
    if (!ok)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap HPET base address");
        return;
    }

    ok = page_manager_unmap_memory((u32)hpet.hpet_table);
    if (!ok)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap HPET table");
        return;
    }
}

struct hpet_driver_t hpet_driver = 
{ 
    .driver_hdr = {
        .name = "HPET",
        .config = &hpet_config,
        .probe = &hpet_probe,
        .disable = &hpet_disable,
        .state = DRIVER_STATE_UNPROBED
    },
    .read = &hpet_read,
    .write = &hpet_write,
    .read32 = &hpet_read32,
    .write32 = &hpet_write32,
};

const struct generic_driver_t* hpet_get_driver()
{
    hpet_driver.mmio_utils = mmio_load_defaults();
    return (struct generic_driver_t*)&hpet_driver;
}