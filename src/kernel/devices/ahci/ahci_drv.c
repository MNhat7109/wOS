#include "ahci_drv.h"
#include "../../pci/pci.h"

void ahci_config();
bool ahci_probe();
u32 ahci_read();
void ahci_write(int pool, u32 value);

generic_driver_t ahci_driver = 
{
    .name = "AHCI",
    .config = &ahci_config,
    .probe = &ahci_probe,
    .read = &ahci_read,
    .write = &ahci_write
};
const generic_driver_t* ahci_get_driver()
{
    return &ahci_driver;
}