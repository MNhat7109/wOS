#include "lapic.h"
#include "lapic_defs.h"
#include "lapic_utils.h"

#include "../cpu.h"
#include <stdbool.h>
#include "../../../paging/paging.h"

void lapic_config(struct generic_driver_t* driver)
{
    struct lapic_driver_t* lapic_self = (struct lapic_driver_t*)driver;
    page_manager_map_memory(lapic_self->__lapic_base, lapic_self->__lapic_base);

    // Enable LAPIC, and set Spurious Interrupt to vector 0xFF
    lapic_write(lapic_self, LAPIC_REG_SVR, LAPIC_SVR_APIC | (0xFF<<0));
    
    // Enable all interrupts
    lapic_write(lapic_self, LAPIC_REG_TPR, 0);
}

void lapic_probe(struct generic_driver_t*)
{
}

void lapic_disable(struct generic_driver_t* driver)
{
    struct lapic_driver_t* lapic_self = (struct lapic_driver_t*)driver;

    bool status = page_manager_unmap_memory(lapic_self->__lapic_base);
    if (!status)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot unmap the LAPIC base address");
        return;
    }
}

struct lapic_driver_t lapic_layer = {
    .driver_hdr = {
        .name = "LAPIC",
        .config = &lapic_config,
        .probe = &lapic_probe,
        .disable = &lapic_disable,
        .state = DRIVER_STATE_UNPROBED
    },
    .read = &lapic_read,
    .write = &lapic_write,
    .get_div_value = &lapic_cook_div_mode
};

const struct lapic_driver_t* lapic_get_driver()
{
    lapic_layer.mmio = mmio_load_defaults();
    return &lapic_layer;
}