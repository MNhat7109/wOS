#include <stdbool.h>
#include "../../stdint.h"
#include "../../stdio.h"

#include "../../paging/paging.h"
#include "../../string/string.h"
#include "../../ktime/ktime.h"
#include "../../memory/memory.h"

#include "ahci_hba.h"
#include "ahci_ports.h"
#include "ahci_config.h"
#include "ahci.h"
#include "ahci_utils.h"
#include "ahci_detect.h"
#include "ahci_interrupts.h"

void ahci_get_device_attributes(struct ahci_driver_t* self)
{
    u16 identify_buffer[256];
    kprintf("AHCI: Drive properties:\n");
    for (u32 i=0;i<self->ports.current_device_count;i++)
    {
        int status, dev_type = ahci_port_get_type(&self->ports.devices[i]);
        switch (dev_type)
        {
        case AHCI_PORT_TYPE_SATA:
            status = ahci_identify(self, &self->ports.devices[i], identify_buffer);
            if (status != AHCI_CMD_SUCCESS)
            {
                // TODO: Maybe ahci_read_error?
                break;
            }
            if (identify_buffer[83] & (1<<10)) 
            ahci_port_set_flag(&self->ports.devices[i], AHCI_DEV_LBA48);
            if (identify_buffer[49] & (1<<9)) 
            ahci_port_set_flag(&self->ports.devices[i], AHCI_DEV_ADDRMODE);
            self->ports.devices[i].max_lba_28 = *(u32*)&identify_buffer[60];
            self->ports.devices[i].max_lba_48 = *(u64*)&identify_buffer[100];

            kprintf("Device %u:\n", i);
            kprintf("   Port no. %u, type: SATA\n", 
                ahci_port_get_number(&self->ports.devices[i]));
            kprintf("   Max User Addressable Sector Count: %u\n"
                , self->ports.devices[i].max_lba_28);
            kprintf("   Max Extended User Addressable Sector Count (LBA48): %u\n"
                , self->ports.devices[i].max_lba_48);
            break;
        }
    }
}

void ahci_config(struct generic_driver_t* driver)
{
    struct ahci_driver_t* ahci_self = (struct ahci_driver_t*)driver;
    kprintf("AHCI: Probing features...\n");
    ahci_probe_features(ahci_self->__abar);
    u32 cap_x = ahci_self->__abar->host_capability_ext;
    
    if ((cap_x>>0)&1)
    {
        kprintf("AHCI: Performing BIOS/OS handoff...");
        hba_perform_bios_os_handoff(ahci_self->__abar);
        kprintf("ok\n");
    }

    ahci_port_setup(&ahci_self->ports, ahci_self->__abar);

    hba_reset(ahci_self->__abar);
    ahci_port_reset_all(&ahci_self->ports, ahci_self->__abar);
    ahci_self->__abar->global_host_ctl |= HBA_GHC_AE;
    ahci_interrupt_setup(ahci_self->__abar, &ahci_self->ports, ahci_self->__irq);

    kprintf("AHCI: Probing ports...\n");
    
    ahci_port_detect(&ahci_self->ports, ahci_self->__abar);
    ahci_get_device_attributes(ahci_self);
}
