#include "ahci_info.h"
#include "../../stdint.h"
#include "../../stdio.h"
#include "../../pci/pci.h"

u32 abar_phys;
u8 interrupt_line;

bool ahci_detect(pci_dev_t* dev)
{
    if (dev->type.class != 0x01 || dev->type.sub != 0x06 || dev->prog_if != 1)
    {
        // It's not an AHCI controller
        //kprintf("AHCI: controller not found\n");
        return false;
    }
    
    if (!dev->mmio_phys_addr) 
    {
        u16 abar_lo = PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x24);
        u16 abar_hi = PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x24+2);
        abar_phys = (u32)(abar_lo | (abar_hi << 16));
        interrupt_line = PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x3C);
    }
    pci_hdr0_t* hdr_mem_space = (pci_hdr0_t*)dev->mmio_phys_addr;
    abar_phys = (hdr_mem_space->bar5 & ~0xF); // BAR5
    interrupt_line = hdr_mem_space->interrupt_line;

    hdr_mem_space->hdr.command |= (1<<1); // Enable Memory Space access 
    hdr_mem_space->hdr.command |= (1<<2); // Enable Bus Mastering 
    hdr_mem_space->hdr.command &= ~(1<<10); // Enable Interrupts
    
    kprintf("AHCI: found controller with class id=0x%x, subclass id=0x%x. ABAR=0x%x, IRQ: %hhu\n", 
        dev->type.class, dev->type.sub, abar_phys, interrupt_line);
    return true;
}

bool ahci_probe()
{
    return PCI_scan(ahci_detect);
}