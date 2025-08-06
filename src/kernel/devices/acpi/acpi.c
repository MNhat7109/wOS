#include "acpi.h"
#include "acpi_config.h"
#include "acpi_utils.h"
#include "acpi_detect.h"

struct acpi_driver_t acpi_dev_driver = {
    .driver_hdr = {
        .name = "ACPI",
        .config = &acpi_config,
        .probe = &acpi_probe,
        .disable = &acpi_disable,
        .state = DRIVER_STATE_UNPROBED
    },
    .get_table = &acpi_find_table,
};

const struct generic_driver_t* acpi_get_driver()
{
    return (struct generic_driver_t*)&acpi_dev_driver;
}