#include "ioapic.h"
#include "ioapic_defs.h"
#include "ioapic_utils.h"

#include <stdbool.h>
#include "../cpu.h"
#include "../../../paging/paging.h"
#include "../../../string/string.h"
#include "../../../stdio.h"

struct ioapic_shared_t ioapic;

void ioapic_probe(struct generic_driver_t*)
{
}

void ioapic_config(struct generic_driver_t* driver)
{
    struct ioapic_driver_t* ioapic_self = (struct ioapic_driver_t*)driver;

    // This time, paging and bitmap allocator is set up
    // will not use the stack, afraid it's gonna explode
    ioapic.ioapic_list = (struct ioapic_info_t*)page_alloc_request();
    ioapic.iso_map = (struct iso_info_t*)page_alloc_request();

    page_manager_map_memory((u32)ioapic.ioapic_list, (u32)ioapic.ioapic_list);
    page_manager_map_memory((u32)ioapic.iso_map, (u32)ioapic.iso_map);
    
    ioapic.current_ioapic_count = 0;
    ioapic.max_gsi_count = 0;
    ioapic.driver = ioapic_self;

    memset(ioapic.ioapic_list, 0, sizeof(struct ioapic_info_t)*MAX_IOAPIC_ENTRIES);
    memset(ioapic.iso_map, 0, sizeof(struct iso_info_t)*MAX_ISO_ENTRIES);
    
    ioapic_self->cpu_dev->scan_from_madt(ioapic_detect);
    if (ioapic.current_ioapic_count == 0)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot find any IOAPICs");
        return;
    }
    ioapic_self->cpu_dev->scan_from_madt(iso_detect);
}

void ioapic_disable(struct generic_driver_t* driver)
{
    u32 base;

    for (u32 i=0;i<ioapic.current_ioapic_count; i++)
    {
        struct ioapic_info_t* ioapic_entry_ptr = &ioapic.ioapic_list[i];
        base = page_manager_unmap_memory(ioapic_entry_ptr->ioapic_base);
        if (!base)
        {
            driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap the IOAPIC base from GSI list");
            return;
        }
    }

    // Deallocate ioapic maps
    base = page_manager_unmap_memory((u32)ioapic.ioapic_list);
    if (!base)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap the IOAPIC GSI list");
        return;
    }
    page_alloc_free(base);

    base = page_manager_unmap_memory((u32)ioapic.iso_map);
    if (!base)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap the ISO GSI map");
        return;
    }
    page_alloc_free(base);
}

struct ioapic_driver_t ioapic_layer = {
    .driver_hdr = {
        .name = "IOAPIC",
        .config = &ioapic_config,
        .probe = &ioapic_probe,
        .disable = &ioapic_disable,
        .state = DRIVER_STATE_UNPROBED
    },
    .read = &ioapic_read,
    .write = &ioapic_write,
    .irq_to_gsi = &ioapic_irq_to_gsi,
    .redirect_gsi = &ioapic_redirect_gsi,
    .cut_gsi = &ioapic_cut_gsi
};

const struct ioapic_driver_t* ioapic_get_driver()
{
    ioapic_layer.mmio = mmio_load_defaults();
    return &ioapic_layer;
}