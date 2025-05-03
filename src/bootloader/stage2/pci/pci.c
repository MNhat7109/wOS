#include "pci.h"
#include "../io/io.h"
#include <stdbool.h>

typedef struct
{
    u8 reg_offset;
    struct
    {
        u8 func_num : 3;
        u8 device_num : 5;
    };
    u8 bus_num;
    struct
    {
        u8 _reserved : 7;
        u8 enable : 1;
    };
} __attribute__((packed)) config_addr_t;

u16 PCI_config_read_word(u8 bus, u8 slot, u8 func, u8 offset)
{
    union {config_addr_t in; u32 out;} config_address;
    config_addr_t* address = &config_address.in;
    address->reg_offset =offset&0xFC;
    address->func_num=func;
    address->device_num=slot;
    address->bus_num=bus;
    address->_reserved=0;
    address->enable=1;

    outl(0xCF8, config_address.out);
    u16 value = (u16)((inl(0xCFC) >> ((offset & 2)*8))&0xFFFF);
    return value;
}

static bool PCI_scan_bus(int bus, pci_dev_func_t callback);

bool PCI_scan(pci_dev_func_t callback)
{
    for (int i = 0 ;i<=0xFF;i++)
    {
        if (PCI_scan_bus(i, callback))
            return true;
    }
    return false;
}

static bool PCI_is_multifunc(int bus, int device)
{
    u16 type = PCI_config_read_word(bus, device, 0, 0xC+2) & 0xFF;
    return (type & 0x80)>>7;
}

static bool PCI_scan_func(int bus, int device, int func, pci_dev_func_t callback);

static bool PCI_scan_bus(int bus,pci_dev_func_t callback)
{
    int dev, func;

    for (dev=0;dev<32;dev++)
    {
        if (!PCI_is_multifunc(bus, dev))
        {
            if (PCI_scan_func(bus, dev, 0, callback)) return true;
            continue;
        }
        for (func=0;func<8;func++)
            if (PCI_scan_func(bus,dev,func, callback)) return true;
    }
    return false;
}

static u16 get_device_type(int bus, int device, int func)
{
    return PCI_config_read_word(bus, device, func, 0x8+2);
}

static bool PCI_scan_func(int bus, int device, int func, pci_dev_func_t callback)
{
    u16 device_id = PCI_config_read_word(bus, device, func, 2);
    if (device_id==0xFFFF) return false;
    u16 vendor_id = PCI_config_read_word(bus, device, func, 0);
    u16 type = get_device_type(bus, device, func);
    u16 class = (type>>8)&0xFF;
    u16 subclass = type&0xFF;

    pci_dev_t device_pack = {.device_id=device_id,.vendor_id=vendor_id,.type=type,.bus=bus,.slot=device,.func=func};
    return callback(&device_pack);
    // kprintf("%x:%x, class=%x,subclass=%x\n",device_id,vendor_id,class,subclass);
}

u8 PCI_get_irq_vector(pci_dev_t* dev)
{
    return (u8)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x3C) & 0xFF);
}