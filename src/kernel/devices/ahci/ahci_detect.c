#include "ahci.h"
#include "ahci_hba.h"
#include "ahci_detect.h"

#include "../../stdio.h"
#include "../../pci/pci.h"
#include "../../string/string.h"
#include "../../paging/paging.h"

#define AHCI_SIG_ATA   0x00000101
#define AHCI_SIG_ATAPI 0xEB140101
#define AHCI_SIG_SEMB  0xC33C0101
#define AHCI_SIG_PM    0x96690101

static struct
{
    hba_memory_t* abar;
    u32 bar5;
    u8 irq;
} ahci_detect;

void ahci_probe_features(hba_memory_t* abar)
{
    u32 cap = abar->host_capability;
    u32 cap_x = abar->host_capability_ext;
    
    kprintf("%x, %x\n", cap, cap_x);

    kprintf("AHCI Capability: Number of ports: %u\n"
        , (cap>>0)&0x1F);
    
    kprintf("AHCI Capability: eSATA Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>5)&1]);
        kprintf("AHCI Capability: Enclosure Bridge Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>6)&1]);

    kprintf("AHCI Capability: Command Completion Coalescing Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>7)&1]);
    kprintf("AHCI Capability: Number of cmd slots: %u\n"
        , (cap>>8)&0x1F);
        
    kprintf("AHCI Capability: Partial State Capable: %s\n"
    , (const char*[]){"no", "yes"}[(cap>>13)&1]);
    kprintf("AHCI Capability: Slumber State Capable: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>14)&1]);
    kprintf("AHCI Capability: PIO Multiple DRQ Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>15)&1]);
        kprintf("AHCI Capability: FIS-based Switching Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>16)&1]);
            kprintf("AHCI Capability: Port Multiplier Support: %s\n"
                , (const char*[]){"no", "yes"}[(cap>>17)&1]);
                
    kprintf("AHCI Capability: AHCI mode only: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>18)&1]);
    
        const char* iss[16] = {"no",};
        iss[1] = "Gen 1.0"; iss[2] = "Gen 2.0"; iss[3] = "Gen 3.0"; 
    kprintf("AHCI Capability: Interface Speed Support: %s\n"
    , iss[(cap>>20)&0xF]);
    kprintf("AHCI Capability: Cmd List Override Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>24)&1]);
    kprintf("AHCI Capability: Activity LED Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>25)&1]);
        kprintf("AHCI Capability: Aggressive Link Power Management Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap>>26)&1]);
    kprintf("AHCI Capability: Staggered Spin-up Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>27)&1]);
        kprintf("AHCI Capability: Mechanical Presence Switch Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>28)&1]);
            kprintf("AHCI Capability: PxSNTF Register Support: %s\n"
                , (const char*[]){"no", "yes"}[(cap>>29)&1]);
    kprintf("AHCI Capability: Native Cmd Queuing Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>30)&1]);
        kprintf("AHCI Capability: 64-bit DMA Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>31)&1]);

            kprintf("AHCI Capability Extended: BIOS/OS Handoff Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>0)&1]);
    
    kprintf("AHCI Capability Extended: NVMHCI Present: %s\n"
        , (const char*[]){"no", "yes"}[(cap_x>>1)&1]);
        kprintf("AHCI Capability Extended: Auto Partial to Slumber Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>2)&1]);
    kprintf("AHCI Capability Extended: Device Sleep Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>3)&1]);
    kprintf("AHCI Capability Extended: Aggressive Device Sleep Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>4)&1]);
    kprintf("AHCI Capability Extended: Device Sleep Entrance from Slumber only: %s\n"
        , (const char*[]){"no", "yes"}[(cap_x>>5)&1]);
}

bool ahci_pci_detect(pci_dev_t* dev)
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
        ahci_detect.bar5 = (u32)(abar_lo | (abar_hi << 16));
        ahci_detect.irq = PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x3C);
        // TODO
    }
    else
    {    
        pci_hdr0_t* hdr_mem_space = (pci_hdr0_t*)dev->mmio_phys_addr;
        ahci_detect.bar5 = (hdr_mem_space->bar5 & ~0xF); // BAR5
        ahci_detect.irq = hdr_mem_space->interrupt_line;

        hdr_mem_space->hdr.command |= (1<<1); // Enable Memory Space access 
        hdr_mem_space->hdr.command |= (1<<2); // Enable Bus Mastering 
        hdr_mem_space->hdr.command &= ~(1<<10); // Enable Interrupts
    }    
    kprintf("AHCI: found controller with class id=0x%x, subclass id=0x%x. ABAR=0x%x, IRQ: %hhu\n", 
        dev->type.class, dev->type.sub, ahci_detect.bar5, ahci_detect.irq);

    u32 abar_virt = page_alloc_request();
    page_manager_map_memory(abar_virt, ahci_detect.bar5);
    ahci_detect.abar = (hba_memory_t*)abar_virt;
    return true;
}

bool ahci_probe(struct generic_driver_t* driver)
{
    struct ahci_driver_t* ahci_self = (struct ahci_driver_t*)driver;
    bool pci_status = PCI_scan(ahci_pci_detect);
    if (!pci_status) return false;

    ahci_self->__abar = ahci_detect.abar;
    ahci_self->__irq = ahci_detect.irq;
    ahci_self->__bar5 = ahci_detect.bar5;
    return true;
}