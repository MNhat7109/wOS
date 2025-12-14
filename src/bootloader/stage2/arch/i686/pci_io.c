#include "../../drivers/pci.h"
#include "io.h"

#define PCI_IO_CONFIG_ADDR_PORT 0xCF8
#define PCI_IO_CONFIG_DATA_PORT 0xCFC

bool pci_io_scan_bus(pci_device_t* device, u8 bus, bool (*callback)(pci_device_t*));
bool pci_io_scan_dev(pci_device_t* device, u8 bus, u8 dev, bool (*callback)(pci_device_t*));
bool pci_io_scan_func(pci_device_t* device, u8 bus, u8 dev, u8 func, bool (*callback)(pci_device_t*));

bool pci_io_scan(pci_device_t* device, bool (*callback)(pci_device_t*))
{
    for (u8 bus=0;bus<=0xFF;bus++)
    {
        if (pci_io_scan_bus(device, bus, callback)) return true;
    }
    return false;
}

u32 pci_io_read(pci_device_t* device, u8 offset)
{
    // Form configuration address
    u32 config_addr = 
    0x80000000 | 
    ((u32)(device->bus&0xFF)<<16) | 
    ((u32)(device->dev&0x1F)<<11) |
    ((u32)(device->func&0x07)<<8) |
    ((u32)offset&0xFC);

    outl(PCI_IO_CONFIG_ADDR_PORT, config_addr);
    return inl(PCI_IO_CONFIG_DATA_PORT);
}

void pci_io_write(pci_device_t* device, u8 offset, u32 value)
{
    // Form configuration address
    u32 config_addr = 
    0x80000000 | 
    ((u32)(device->bus&0xFF)<<16) | 
    ((u32)(device->dev&0x1F)<<11) |
    ((u32)(device->func&0x07)<<8) |
    ((u32)offset&0xFC);

    outl(PCI_IO_CONFIG_ADDR_PORT, config_addr);
    outl(PCI_IO_CONFIG_DATA_PORT, value);
}

bool pci_io_scan_bus(pci_device_t* device, u8 bus, bool (*callback)(pci_device_t*))
{
    for (u8 dev=0;dev<32;dev++)
    {
        if (pci_io_scan_dev(device, bus, dev, callback)) return true;
    }
    return false;
}

bool pci_io_scan_dev(pci_device_t* device, u8 bus, u8 dev, bool (*callback)(pci_device_t*))
{
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);

    u32 config_addr_bd = (u32)0x80000000 | (fixed_bus<<16) | (fixed_dev<<11);

    u32 dw1 = config_addr_bd | 0; // Get the first DWORD in the config space
    outl(PCI_IO_CONFIG_ADDR_PORT, dw1);
    u16 ven_id = inl(PCI_IO_CONFIG_DATA_PORT);

    u32 dw3 = config_addr_bd | 0xC; // Get the third DWORD in the config space
    outl(PCI_IO_CONFIG_ADDR_PORT, dw3);
    // Grab the upper 16 bits
    u8 hdr_type = (inl(PCI_IO_CONFIG_DATA_PORT)>>16)&0xFF;

    if (ven_id == 0xFFFF) return false;

    bool func_mapping;
    if (!(hdr_type & PCI_HDR_TYPE_MULTIFUNC))
    {
        func_mapping = pci_io_scan_func(device, bus, dev, 0, callback);
        return func_mapping;
    }

    for (u8 func=0;func<8;func++)
    {
        func_mapping = pci_io_scan_func(device, bus, dev, func, callback);
        if (func_mapping) return true;

        if (func==0) return false;
    }
    return false;
}

bool pci_io_scan_func(pci_device_t* device, u8 bus, u8 dev, u8 func, bool (*callback)(pci_device_t*))
{
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);
    u32 fixed_func = (u32)(func&0x07);

    u32 config_addr_bdf = (u32)0x80000000 | (fixed_bus<<16) | (fixed_dev<<11) | (fixed_func<<8);

    u32 dw1 = config_addr_bdf | 0; // Get the first DWORD in the config space
    outl(PCI_IO_CONFIG_ADDR_PORT, dw1);
    u16 ven_id = inl(PCI_IO_CONFIG_DATA_PORT);

    if (ven_id == 0xFFFF) return false;

    device->bus = bus;
    device->dev = dev;
    device->func = func;

    return callback(device);
}
