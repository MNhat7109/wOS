#include "pci_utils.h"
#include "pci.h"
#include "pci_io.h"
#include "pci_mm.h"
#include "pci_defs.h"
 
extern struct pci_shared_t pci;

bool pci_scan_devices(struct pci_driver_t* self, bool (*callback)(struct pci_driver_t*))
{
    if (pci.mmio_available & PCI_MCFG_PRESENT)
        return pci_mm_scan(self, callback);
    return pci_io_scan(self, callback);
}

u32 pci_read(struct pci_driver_t* self, u8 offset)
{
    // Ensure DWORD-aligned offset (PCI config space is 32-bit aligned)
    offset&=0xFC;

    if (pci.mmio_available & PCI_MCFG_PRESENT)
        return pci_mm_read(self, offset);
    return pci_io_read(self,offset);
}

void pci_write(struct pci_driver_t* self, u8 offset, u32 value)
{
    // Ensure DWORD-aligned offset (PCI config space is 32-bit aligned)
    offset&=0xFC;
    
    if (pci.mmio_available & PCI_MCFG_PRESENT)
        pci_mm_write(self, offset, value);
    else pci_io_write(self, offset, value);
}