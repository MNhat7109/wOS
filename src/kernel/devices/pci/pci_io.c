#include "pci.h"
#include "pci_io.h"
#include "pci_defs.h"

#define PCI_IO_CONFIG_ADDR_PORT 0xCF8
#define PCI_IO_CONFIG_DATA_PORT 0xCFC

extern struct pci_shared_t pci;

// Forward declarations first

static bool pci_io_scan_bus(
    struct pci_driver_t* self, 
    u8 bus, 
    bool (*callback)(struct pci_driver_t*));
static bool pci_io_scan_dev(
    struct pci_driver_t* self, 
    u8 bus, u8 dev, 
    bool (*callback)(struct pci_driver_t*));
static bool pci_io_scan_func(
    struct pci_driver_t* self, 
    u8 bus, u8 dev, u8 func, 
    bool (*callback)(struct pci_driver_t*));

/*-- PROTOTYPE DEFINITION --*/

bool pci_io_scan(struct pci_driver_t* self, bool (*callback)(struct pci_driver_t*))
{
    for (u8 bus=0;bus<=0xFF;bus++)
    {
        if (pci_io_scan_bus(self, bus, callback)) return true;
    }
    return false;
}

u32 pci_io_read(struct pci_driver_t* self, u8 offset)
{
    // NOTE: This function is written with the assumption that offset has
    // already been divisible by 4

    // Form configuration address
    u32 config_addr = 
    0x80000000 | 
    ((u32)(self->bus&0xFF)<<16) | 
    ((u32)(self->dev&0x1F)<<11) |
    ((u32)(self->func&0x07)<<8) |
    ((u32)offset&0xFC);

    // Let the CPU sip in the address
    pci.pio_utils.writel(PCI_IO_CONFIG_ADDR_PORT, config_addr);

    // And finally, get the data
    return pci.pio_utils.readl(PCI_IO_CONFIG_DATA_PORT);
}

void pci_io_write(struct pci_driver_t* self, u8 offset, u32 value)
{
    // NOTE: This function is written with the assumption that offset has
    // already been divisible by 4

    // Form configuration address
    u32 config_addr = 
    0x80000000 | 
    ((u32)(self->bus&0xFF)<<16) | 
    ((u32)(self->dev&0x1F)<<11) |
    ((u32)(self->func&0x07)<<8) |
    ((u32)offset&0xFC);

    // Let the CPU sip in the address
    pci.pio_utils.writel(PCI_IO_CONFIG_ADDR_PORT, config_addr);

    // Write your desired value in
    pci.pio_utils.writel(PCI_IO_CONFIG_DATA_PORT, value);
}

/*-- INTERNAL FUNCTION DEFINITION --*/

static bool pci_io_scan_bus(
    struct pci_driver_t* self, 
    u8 bus, 
    bool (*callback)(struct pci_driver_t*))
{
    for (u8 dev=0;dev<32;dev++)
    {
        if (pci_io_scan_dev(self, bus, dev, callback)) return true;
    }
    return false;
}

static bool pci_io_scan_dev(
    struct pci_driver_t* self, 
    u8 bus, u8 dev, 
    bool (*callback)(struct pci_driver_t*))
{
    // Do a little fixing, in case our future dev is too tired
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);

    u32 config_addr_bd = (u32)0x80000000 | (fixed_bus<<16) | (fixed_dev<<11);

    // NOTE: Reason for PIO API instead of PCI API: Incomplete bus, device, function
    // Essentially the same reason as in the MMIO functions.

    u32 dw1 = config_addr_bd | 0; // Get the first DWORD in the config space
    pci.pio_utils.writel(PCI_IO_CONFIG_ADDR_PORT, dw1);
    u16 ven_id = pci.pio_utils.readl(PCI_IO_CONFIG_DATA_PORT);

    u32 dw3 = config_addr_bd | 0xC; // Get the third DWORD in the config space
    pci.pio_utils.writel(PCI_IO_CONFIG_ADDR_PORT, dw3);
    // Grab the upper 16 bits
    u8 hdr_type = (pci.pio_utils.readl(PCI_IO_CONFIG_DATA_PORT)>>16)&0xFF;

    if (ven_id == 0xFFFF) return false;

    bool func_mapping;
    if (!(hdr_type & PCI_HDR_TYPE_MULTIFUNC))
    {
        func_mapping = pci_io_scan_func(self, bus, dev, 0, callback);
        return func_mapping;
    }

    for (u8 func=0;func<8;func++)
    {
        func_mapping = pci_io_scan_func(self, bus, dev, func, callback);
        if (func_mapping) return true;

        if (func==0) return false;
    }
    return false;
}

static bool pci_io_scan_func(
    struct pci_driver_t* self, 
    u8 bus, u8 dev, u8 func, 
    bool (*callback)(struct pci_driver_t*))
{
    // Do a little fixing, in case our future dev is too tired
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);
    u32 fixed_func = (u32)(func&0x07);

    u32 config_addr_bdf = (u32)0x80000000 | (fixed_bus<<16) | (fixed_dev<<11) | (fixed_func<<8);

    // NOTE: Reason for PIO API instead of PCI API: Incomplete bus, device, function
    // Essentially the same reason as in the MMIO functions.

    u32 dw1 = config_addr_bdf | 0; // Get the first DWORD in the config space
    pci.pio_utils.writel(PCI_IO_CONFIG_ADDR_PORT, dw1);
    u16 ven_id = pci.pio_utils.readl(PCI_IO_CONFIG_DATA_PORT);

    if (ven_id == 0xFFFF) return false;

    // NOTE: Almost forgot, save the bus, device, and function for future use
    self->bus = bus;
    self->dev = dev;
    self->func = func;

    return callback(self);
}