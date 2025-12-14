#include "../../drivers/pci.h"

bool pci_io_scan(pci_device_t* device, bool (*callback)(pci_device_t*)); 
u32 pci_io_read(pci_device_t* device, u8 offset);
void pci_io_write(pci_device_t* device, u8 offset, u32 value);

bool pci_scan(pci_device_t* device, bool (*callback)(pci_device_t*))
{
    return pci_io_scan(device, callback);
}

u32 pci_read(pci_device_t* device, u8 offset)
{
    offset &= 0xFC; // DWORD-aligned offset
    return pci_io_read(device, offset);
}

void pci_write(pci_device_t* device, u8 offset, u32 value)
{
    offset &= 0xFC; // DWORD-aligned offset
    pci_io_write(device,offset,value);
}