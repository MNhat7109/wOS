#include "pci.h"
#include "pci_defs.h"
#include "pci_mm.h"

#include "../../paging/paging.h"

extern struct pci_shared_t pci;

// Forward declare every internal (should be static) functions first, 
// or GCC will nag on me and won't work with me no more.

// Also, doing this will help me and future devs 
// who are unfortunate enough to work on my code focus on the functions that are
// declared in the header file.

static bool pci_mm_scan_bus(struct pci_driver_t* self, 
    u8 bus, 
    bool (*callback)(struct pci_driver_t*));
static bool pci_mm_scan_dev(struct pci_driver_t* self, 
    u8 bus, u8 dev, 
    bool (*callback)(struct pci_driver_t*));
static bool pci_mm_scan_func(struct pci_driver_t* self, 
    u8 bus, u8 dev, u8 func,
    bool (*callback)(struct pci_driver_t*));

bool pci_mm_scan(struct pci_driver_t* self, bool (*callback)(struct pci_driver_t*))
{
    for (u32 i=0;i<pci.mcfg_entries;i++)
    {
        pci_dev_cfg_t* dev_config 
        = (pci_dev_cfg_t*)((u32)pci.mcfg+sizeof(mcfg_t)+i*sizeof(pci_dev_cfg_t));

        page_manager_map_memory((u32)dev_config, (u32)dev_config);
        for (u8 bus=dev_config->bus_start; bus<=dev_config->bus_end;bus++)
        {
            page_manager_map_memory((u32)dev_config->base, (u32)dev_config->base);
            self->ecam_base = dev_config->base;
            bool bus_mapping = pci_mm_scan_bus(self, bus, callback);
            page_manager_unmap_memory((u32)dev_config->base);

            if (bus_mapping)
            {
                page_manager_unmap_memory((u32)dev_config);
                return true;
            }
        }
        page_manager_unmap_memory((u32)dev_config);
    }
    return false;
}

u32 pci_mm_read(struct pci_driver_t* self, u8 offset)
{
    // NOTE: This function is written with the assumption that offset has
    // already been divisible by 4

    // Assumed that the MMIO base has been mapped beforehand
    // in the scan() function.

    u32 mmio_base = 
    self->ecam_base + 
    ((self->bus&0xFF)<<20) + 
    ((self->dev&0x1F)<<15) + 
    ((self->func&0x07)<<12);

    return pci.mmio_utils.readl(mmio_base, (offset&0xFC));
}

void pci_mm_write(struct pci_driver_t* self, u8 offset, u32 value)
{
    // NOTE: This function is written with the assumption that offset has
    // already been divisible by 4

    // Assumed that the MMIO base has been mapped beforehand
    // in the scan() function.

    u32 mmio_base = 
    self->ecam_base + 
    ((self->bus&0xFF)<<20) + 
    ((self->dev&0x1F)<<15) + 
    ((self->func&0x07)<<12);

    pci.mmio_utils.writel(mmio_base, (offset&0xFC), value);
}

/*-- INTERNAL FUNCTION DEFINITION --*/

static bool pci_mm_scan_bus(struct pci_driver_t* self, 
    u8 bus, 
    bool (*callback)(struct pci_driver_t*))
{
    for (u8 dev=0;dev<32;dev++)
    {
        if (pci_mm_scan_dev(self, bus, dev, callback))
        return true;
    }
    return false;
}


static bool pci_mm_scan_dev(struct pci_driver_t* self, 
    u8 bus, u8 dev, 
    bool (*callback)(struct pci_driver_t*)) 
{
    // Do a little fixing, in case our future dev is too tired
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);

    u32 ecam_bd = self->ecam_base + (fixed_bus<<20) + (fixed_dev<<15);
    page_manager_map_memory(ecam_bd, ecam_bd);
    
    // NOTE: Now you'll ask, why do I use MMIO API's readl() instead of
    // the PCI MMIO driver's read()?
    // Well, PCI's read() will only be a to-go, if we are able to find
    // the right bus, device, and function beforehand.

    // So please please please, if you're a future dev, do NOT swap this 
    // code out for read(). Why? Go read the first comment.

    u16 ven_id = pci.mmio_utils.readl(ecam_bd, 0);
    u8 hdr_type = pci.mmio_utils.readl(ecam_bd, 0xE);
    
    page_manager_unmap_memory(ecam_bd); // Clean it up. Or else I'll forget and it'll become a black hole.
    
    // NOTE: Also, if you want to do anything that involves tampering with
    // PCI config space fields, do so INSIDE THE UNMAP FUNCTION.
    // Trust me. It'll save you lots of time and effort poured into
    // meaningless debugging.

    if (ven_id == 0xFFFF) return false; // Vendor ID = 0xFFFF ==> No devices present

    bool func_mapping;
    if (!(hdr_type & PCI_HDR_TYPE_MULTIFUNC))
    {
        func_mapping = pci_mm_scan_func(self, bus, dev, 0, callback);
        return func_mapping;
    }

    for (u8 func=0;func<8;func++)
    {
        func_mapping = pci_mm_scan_func(self, bus, dev, func, callback);
        if (func_mapping) return true;

        // If function 0 is missing (vendor ID == 0xFFFF),
        // the device slot is empty, so skip scanning other functions.
        if (func==0) return false;
    }
    return false;
}

static bool pci_mm_scan_func(struct pci_driver_t* self, 
    u8 bus, u8 dev, u8 func,
    bool (*callback)(struct pci_driver_t*))
{
    // Do a little fixing, in case our future dev is too tired
    u32 fixed_bus = (u32)(bus&0xFF);
    u32 fixed_dev = (u32)(dev&0x1F);
    u32 fixed_func = (u32)(func&0x07);

    u32 ecam_bdf = self->ecam_base + (fixed_bus<<20) + (fixed_dev<<15) + (fixed_func<<12);
    page_manager_map_memory(ecam_bdf, ecam_bdf);

    u16 ven_id = pci.mmio_utils.readl(ecam_bdf, 0);
    
    if (ven_id == 0xFFFF)
    {
        page_manager_unmap_memory(ecam_bdf); 
        // At this point, cleaning up your turd before dying is the best option. 
        return false; // Vendor ID = 0xFFFF ==> No functions present
    }

    // NOTE: Almost forgot, save the bus, device, and function for future use
    self->bus = bus;
    self->dev = dev;
    self->func = func;

    bool cb_status = callback(self); // Now, it is all up to the client's preference of detection.

    page_manager_unmap_memory(ecam_bdf); // Clean up time!
    return cb_status;
}
