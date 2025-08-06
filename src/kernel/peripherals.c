#include "peripherals.h"
#include "devices/driver.h"
#include "hal/hal.h"
#include "stdio.h"

struct generic_driver_t* pci_device; 

bool peripherals_init()
{    
    // Set up PCI
    pci_device = driver_get("PCI");
    if (!pci_device)
    {
        kprintf("Kernel: PCI driver not found\n");
        return false;
    }
    if (!driver_run(pci_device))
    {
        kprintf("Kernel: PCI driver failed to start\n");
        driver_unload("PCI");
        return false;
    }
    return true;
}