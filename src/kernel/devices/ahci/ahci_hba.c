#include "ahci_hba.h"
#include "../../stdio.h"
#include "../../ktime/ktime.h"

void hba_setup_cmd_hdr(hba_cmd_hdr_t* cmd_hdr, int slot, u32 fis_length, u32 prdt_length, u32 direction_write)
{
    cmd_hdr+=slot;
    cmd_hdr->cmd_fis_length = fis_length;
    cmd_hdr->write = direction_write;
    cmd_hdr->prdt_length = prdt_length;
}

void hba_setup_prdt(hba_cmd_table_t* cmd_table, int index, void* buffer, u32 byte_count)
{
    cmd_table->prdt_entries[index].data_base_addr_lo = (u32)buffer;
    cmd_table->prdt_entries[index].data_byte_cnt = byte_count -1;
    cmd_table->prdt_entries[index].interrupt = 1;
}

void hba_reset(hba_memory_t* abar)
{
    abar->global_host_ctl |= HBA_GHC_HR;
    int spin = 1000;
    while (spin>0)
    {
        if (!(abar->global_host_ctl & HBA_GHC_HR)) break;
        sleep(1);
        spin--;
    }
}

bool hba_port_fis_start(hba_port_t* port)
{
    int spin=100;
    while (spin)
    {
        if (port->command_status & (1<<14))
        {
            sleep(1);
            spin--;
        }
        break;
    }
    
    if (spin ==0) return false;
    port->command_status |= (1<<4);
    return true;
}

bool hba_port_fis_stop(hba_port_t* port)
{
    port->command_status &= ~(1<<4);
    int spin=100;
    while (spin)
    {
        if (port->command_status & (1<<14))
        {
            sleep(1);
            spin--;
        }
        break;
    }

    if (spin ==0) 
        return false;
    return true;
}

bool hba_port_cmd_start(hba_port_t* port)
{
    int spin=100;
    while (spin)
    {
        if (port->command_status & (1<<15))
        {
            sleep(1);
            spin--;
        }
        break;
    }
    
    if (spin ==0) return false;
    port->command_status |= (1<<0);
    return true;
}

bool hba_port_cmd_stop(hba_port_t* port)
{
    port->command_status &= ~(1<<0);
    int spin=100;
    while (spin)
    {
        if (port->command_status & (1<<15))
        {
            sleep(1);
            spin--;
        }
        break;
    }

    if (spin ==0) 
        return false;
    return true;
}

bool hba_port_shutdown(hba_port_t* port)
{
    if (!hba_port_fis_stop(port))
    {
        kprintf("AHCI: Failed to stop FIS engine!\n");
        return false;
    }
    if (!hba_port_cmd_stop(port))
    {
        kprintf("AHCI: Failed to stop CMD engine!\n");
        return false;
    }
    return true;
}

bool hba_port_startup(hba_port_t* port)
{
    if (!hba_port_cmd_start(port))
    {
        kprintf("AHCI: Failed to start CMD engine!\n");
        return false;
    }
    if (!hba_port_fis_start(port))
    {
        kprintf("AHCI: Failed to start FIS engine!\n");
        return false;
    }
    return true;
}

bool hba_port_reset(hba_port_t* port)
{
    bool status = hba_port_cmd_stop(port);
    if (!status) return false;
    sleep(400);

    u8 det = (1<<0);
    port->sata_control = (port->sata_control&~(0xF))|det; // Set DET to 0x1
    sleep(10);
    port->sata_control &= ~(0xF);

    while (1)
    {
        if ((port->sata_status & 0xF) == 3) break;
    }
    port->sata_error = (u32)-1;
    status = hba_port_cmd_start(port);
    if (!status) return false;
    return true;
}

u32 hba_port_type_detect(hba_port_t* port)
{
    u32 status = port->sata_status;
    u8 det = status & 0xF;
    u8 ipm = (status >> 8) & 0xF;

    // IPM field = 0x1 -> Device is Active.
    // DET field = 0x3 -> Device and its physical layer (PHY) communication is present.
    // In order for an AHCI devices to be attached, both of these conditions must be met.
    if (det != 0x3 || ipm != 0x1) return 0;

    // The device is attached, get the port type from its signature
    u32 signature = port->signature;
    return signature;
}

bool hba_perform_bios_os_handoff(hba_memory_t* abar)
{
    int spin=100;
    abar->bios_os_handoff_ctl_sts |= HBA_BOHC_OOS;

    while ((abar->bios_os_handoff_ctl_sts & HBA_BOHC_BOS) && spin)
    {
        sleep(1);
        spin--;
    }

    if (spin==0) return false;

    spin=2000;
    while (spin && (abar->bios_os_handoff_ctl_sts & HBA_BOHC_BB)) 
    {
        sleep(1);
        spin--;
    }

    if (spin==0) return false;
    return true; 
}