#include "ahci.h"
#include "ahci_config.h"
#include "ahci_detect.h"
#include "ahci_utils.h"

struct ahci_driver_t ahci_driver = 
{
    .driver_hdr = {
        .name = "AHCI",
        .config = &ahci_config,
        .probe = &ahci_probe,
    },
    .read = ahci_read_sectors,
    .write = ahci_write_sectors
};
const struct generic_driver_t* ahci_get_driver()
{
    return (struct generic_driver_t*)&ahci_driver;
}