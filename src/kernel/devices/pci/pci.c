#include "pci.h"
#include "pci_defs.h"
#include "pci_utils.h"

#include "../acpi/acpi.h"
#include "../../stdio.h"
#include "../../paging/paging.h"

struct pci_shared_t pci;

void pci_setup(struct generic_driver_t* driver)
{
    if (pci.mmio_available & PCI_MCFG_PRESENT)
    {
        page_manager_map_memory((u32)pci.mcfg, (u32)pci.mcfg);
        pci.mmio_utils = mmio_load_defaults();

        pci.mcfg_entries = (pci.mcfg->table_hdr.length-sizeof(mcfg_t))/sizeof(pci_dev_cfg_t);
        return;
    }
    pci.pio_utils = pio_load_defaults();
}

void pci_probe(struct generic_driver_t* driver)
{
    // Start off all optimistic
    pci.mmio_available = PCI_ACPI_PRESENT | PCI_MCFG_PRESENT;
    
    // Get ACPI driver
    pci.acpi_dev = (struct acpi_driver_t*)driver_get("ACPI");
    if (!pci.acpi_dev)
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE, 
            "ACPI driver not found. MMIO addressing will be disabled.");
        // Turn everything off
        pci.mmio_available &= ~PCI_ACPI_PRESENT;
        pci.mmio_available &= ~PCI_MCFG_PRESENT;
    }
    if (!driver_run((struct generic_driver_t*)pci.acpi_dev))
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE, 
            "Failed to start ACPI driver. MMIO addressing will be disabled.");
        // Turn everything off
        pci.mmio_available &= ~PCI_ACPI_PRESENT;
        pci.mmio_available &= ~PCI_MCFG_PRESENT;
    }

    // Grab MCFG for MMIO addressing
    pci.mcfg = (mcfg_t*)pci.acpi_dev->get_table(pci.acpi_dev, "MCFG");
    if (!pci.mcfg)
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE, 
            "MCFG table not found. MMIO addressing will be disabled.");
        pci.mmio_available &= ~PCI_MCFG_PRESENT;
    }
}

void pci_disable(struct generic_driver_t*driver)
{
    if (pci.mmio_available & PCI_MCFG_PRESENT)
    {
        bool ok = page_manager_unmap_memory((u32)pci.mcfg);
        if (!ok)
        {
            driver_log_state(driver, DRIVER_LOG_ERROR, 
                "Cannot unmap MCFG table");
            return;
        }
    }
}

struct pci_driver_t pci_dev_driver = {
    .driver_hdr = {
        .name = "PCI",
        .config = &pci_setup,
        .disable = &pci_disable,
        .probe = &pci_probe,
        .state = DRIVER_STATE_UNPROBED
    },
    .scan = &pci_scan_devices,
    .read = &pci_read,
    .write = &pci_write
};

const struct generic_driver_t* pci_get_driver()
{
    return (struct generic_driver_t*)&pci_dev_driver;
}