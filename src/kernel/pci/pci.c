#include "pci.h"

mcfg_t* acpi_mcfg_table = NULL;

bool PCI_scan_io(pci_dev_func_t callback);
bool PCI_scan_mm(mcfg_t* mcfg_table, pci_dev_func_t callback);

void PCI_init(mcfg_t* mcfg_table)
{
    acpi_mcfg_table = mcfg_table;
}

bool PCI_scan(pci_dev_func_t callback)
{
    // Basically if the MCFG table is NULL, we scan PCI devices the old-fashioned way
    if (!acpi_mcfg_table) return PCI_scan_io(callback);
    return PCI_scan_mm(acpi_mcfg_table, callback);
}