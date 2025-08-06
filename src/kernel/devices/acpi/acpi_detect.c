#include "acpi_detect.h"
#include "../driver.h"
#include "acpi.h"
#include "../../stdint.h"
#include <stdbool.h>
#include "../../boot.h"

bool acpi_rsdp_find(struct generic_driver_t* driver)
{
    struct acpi_driver_t* acpi_self = (struct acpi_driver_t*)driver;
    if (bootloader_info->sdp)
    {
        acpi_self->rxsdp = bootloader_info->sdp;
        return true;
    }
    return false;
}

void acpi_probe(struct generic_driver_t* driver)
{
    if (!acpi_rsdp_find(driver))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Cannot find bootloader RSDP/XSDP");
        return;
    }
}