#include "ahci_detect.h"
#include "ahci_shared.h"
#include "ahci_defs.h"
#include "ahci_hw.h"

#include "../../stdio.h"
#include "../pci/pci.h"
#include "../../string/string.h"
#include "../../paging/paging.h"


#define AHCI_SIG_ATA   0x00000101
#define AHCI_SIG_ATAPI 0xEB140101
#define AHCI_SIG_SEMB  0xC33C0101
#define AHCI_SIG_PM    0x96690101

extern struct ahci_shared_t ahci_shared;

static struct
{
    ahci_controller_t current_ctl;
} ahci_ctl_data;

static bool ahci_pci_detect(struct pci_driver_t* self);

bool ahci_detect_controllers()
{
    ahci_shared.ctl_count = 0;

    // This function scans in a loop, so all our needed information will be filled
    // This only fails if there's no controllers to begin with
    if (!ahci_shared.pci_dev->scan(ahci_shared.pci_dev, ahci_pci_detect))
        return false;
    
    // At this point, the base will be all after the controller elements
    return true;
}

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

/*-- STATIC FUNCTIONS --*/

static bool ahci_pci_detect(struct pci_driver_t* self)
{
    struct generic_driver_t* driver = (struct generic_driver_t*)self;

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Searching for AHCI controller...");

    // Get Class, Subclass and Prog IF
    u32 pci_offset8 = self->read(self, 0x08);
    u8 class = pci_offset8 >> 24;
    u8 sub = pci_offset8 >> 16;
    u8 prog_if = pci_offset8 >> 8;

    if (class != 0x01 || sub != 0x06 || prog_if != 1)
    {
        // It's not an AHCI controller
        driver_log_state(driver, DRIVER_LOG_WARN, "Skipping device: Not an eligible AHCI controller");
        return false;
    }

    // Enable Memory Access, Bus Mastering
    u16 curr_cmd = (u16)(self->read(self, 0x04) & 0xFFFF);
    curr_cmd |= (1<<1); // Memory Space Response
    curr_cmd |= (1<<2); // DMA Bus Mastering

    // Get BAR5
    u32 bar5 = self->read(self, 0x24);

    // Interrupt number handling

    // TODO: Optionally check for MSI/MSI-X support, but we'll skip this for now
    u8 int_line = (u8)(self->read(self, 0x3C) & 0xFF);
    ahci_ctl_data.current_ctl.interrupt_num = int_line;
    curr_cmd &= ~(1<<10); // Clear the Interrupt Disable bit
    
    // Write everything back
    ahci_ctl_data.current_ctl.abar = (hba_memory_t*)(bar5 & ~0xF);
    self->write(self, 0x04, curr_cmd);

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Found AHCI controller with specs below:");
    kprintf("%x:%x.%x, class id=0x%x, subclass id=0x%x. ABAR=0x%x, IRQ: %hhu\n", 
        self->bus, self->dev, self->func, class, sub, 
        ahci_ctl_data.current_ctl.abar, 
        ahci_ctl_data.current_ctl.interrupt_num);

    // Bound check
    if (ahci_shared.ctl_count >= AHCI_CONFIG_MAX_CTL)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Client max controller count exceeded. Skipping...");
        return false;
        // Returning false to signal the PCI driver to skip the other mess
    }

    // Copy everything from the current_ctl to our container

    // Save the needed fields
    // We need to explicitly copy every field to the pointer, not referencing
    // As the current_ctl can change every time the pci_scan is called with this callback
    ahci_controller_t* ctl_ptr = &ahci_shared.controllers[ahci_shared.ctl_count];
    ctl_ptr->abar = ahci_ctl_data.current_ctl.abar;
    ctl_ptr->interrupt_num = ahci_ctl_data.current_ctl.interrupt_num;
    ctl_ptr->device_count = 0; // Just in case
    
    // Map ABAR memory
    for (u32 i=0;i<AHCI_CONFIG_MAX_ABAR_PAGE;i++)
        page_manager_map_memory((u32)ctl_ptr->abar+i*PAGE_SIZE, (u32)ctl_ptr->abar+i*PAGE_SIZE);
    ahci_shared.ctl_count++;
    return true;
}