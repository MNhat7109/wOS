#include "pci.h"
#include "../paging/paging.h"
#include "../stdio.h"

typedef struct
{
    u64 base;
    u16 group_number;
    u8 bus_start;
    u8 bus_end;
    u32 reserved;
} __attribute__((packed)) pci_dev_cfg_t;

bool PCI_scan_func(u32 base, u8 func, pci_dev_func_t callback)
{
    u32 func_offset = func << 12;
    u32 func_addr = base+func_offset;
    page_manager_map_memory(func_addr, func_addr);

    pci_dev_hdr_t* dev_hdr = (pci_dev_hdr_t*)func_addr;
    
    if (dev_hdr->device_id == 0 || dev_hdr->device_id == 0xFFFF)
        return false;

    pci_dev_t dev_pack =
    {
        .device_id = dev_hdr->device_id,
        .vendor_id = dev_hdr->vendor_id,
        .type = {.sub = dev_hdr->sub, .class = dev_hdr->class},
        .bus = (func_addr >> 20)&0xFF,
        .slot = (func_addr >> 15)&0xFF,
        .func = (func_addr >> 12)&0xFF,
        .prog_if = dev_hdr->prog_if,
        .mmio_phys_addr = func_addr
    };

    return callback(&dev_pack);
}

bool PCI_scan_device(u32 base, u8 device, pci_dev_func_t callback)
{
    u32 dev_offset = device << 15;
    u32 dev_addr = base+dev_offset;
    page_manager_map_memory(dev_addr, dev_addr);

    pci_dev_hdr_t* dev_hdr = (pci_dev_hdr_t*)dev_addr;
    u8 hdr_type = dev_hdr->hdr_type;
    bool is_multi_func = (hdr_type & 0x80) >> 7;

    if (!is_multi_func)
    {
        if (PCI_scan_func(dev_addr, 0, callback)) return true;
        return false;
    }


    for (u8 func=0; func<8; func++)
        if (PCI_scan_func(dev_addr, func, callback)) return true;
    return false;
}

bool PCI_scan_bus(u32 base, u8 bus, pci_dev_func_t callback)
{
    u32 bus_offset = bus << 20;
    u32 bus_addr = base+bus_offset;
    page_manager_map_memory(bus_addr, bus_addr);
    
    pci_dev_hdr_t* dev_hdr = (pci_dev_hdr_t*)bus_addr;
    
    for (u8 dev=0; dev<32; dev++)
        if (PCI_scan_device(bus_addr, dev, callback)) return true;
    return false;
}

bool PCI_scan_mm(mcfg_t* mcfg_table, pci_dev_func_t callback)
{
    u32 entries = (mcfg_table->table_hdr.length - sizeof(mcfg_t))/sizeof(pci_dev_cfg_t);

    for (u32 i=0;i<entries;i++)
    {
        pci_dev_cfg_t* dev_conf = (pci_dev_cfg_t*)((u32)mcfg_table + sizeof(mcfg_t)+sizeof(pci_dev_cfg_t)*i);
        //kprintf("%x\n", dev_conf->bus_end);
        for (u8 bus = dev_conf->bus_start; bus <= dev_conf->bus_end; bus++)
        {
            if (PCI_scan_bus(dev_conf->base, bus, callback))
            {
                return true;
            }
        }
    }
    return false;
}