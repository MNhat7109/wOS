#include "ahci_hw.h"
#include "ahci_defs.h"

#include "../../stdio.h"
#include "../../time.h"
#include "../../string/string.h"

bool ahci_prepare_cmd(hba_port_t* port, int slot, u8 cmd, u16 count, bool use_lba, bool write, u64 lba, void* buffer)
{
    if (!buffer || count==0) return false;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)port->cmd_list_base_lo;

    // PRDT size = 8K each, 1 sector = 512 bytes
    // => 1 PRDT = 16 sectors
    // => (Count) sectors = (Count)/16 = (Count>>4) (BITWISE!)
    // The "-1", "+1" is to calculate for the extra PRDT entry, 
    // whenever there's not enough sectors to make a full 8K entry
    u16 prdtl = ((count-1)>>4)+1;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32),prdtl,write);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

    u32 buffer_addr = (u32)buffer;
    u16 i=0;
    
    // We set up (prtdl-1) PRDTs with size of 8K, the leftovers will be saved for the last PRDT entry 
    for (;i<prdtl-1;i++)
    {
        hba_setup_prdt(cmd_table, i, (void*)buffer_addr, 8192);
        buffer_addr+=8192;
        count-=16; // Each sector occupies 512 bytes, so decreasing it by 16 => total size occupied = 16*512 = 8192 bytes
    }
    // At this point, the leftover bytes will be set up as another PRDT entry
    hba_setup_prdt(cmd_table, i, (void*)buffer_addr, (count<<9)); // Count << 9 => Count * (512 bytes per sector)
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;

    // To check if the LBA sits in the LBA48 category, check if the high DWORD is zero or not.
    // If not zero, it's LBA48, otherwise, it's LBA28.
    u32 lba_hi = (lba>>32)&0xFFFFFFFF;
    fis_reg_h2d_setup(cmd_fis, cmd, lba, count, (lba_hi!=0));

    return true;
}

bool ahci_wait_tfd(hba_port_t* port)
{
    int spin=0;
    while ((port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000) spin++;
    if (spin == 1000000) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/* HBA UTILITIES*/
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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
        usleep(1);
        spin--;
    }

    kprintf("Reset done\n");
}

bool hba_port_fis_start(hba_port_t* port)
{
    int spin=100;
    while (spin)
    {
        if (port->command_status & (1<<14))
        {
            usleep(1);
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
            usleep(1);
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
            usleep(1);
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
            usleep(1);
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
    usleep(400);

    u8 det = (1<<0);
    port->sata_control = (port->sata_control&~(0xF))|det; // Set DET to 0x1
    usleep(10);
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
        usleep(1);
        spin--;
    }

    if (spin==0) return false;

    spin=2000;
    while (spin && (abar->bios_os_handoff_ctl_sts & HBA_BOHC_BB)) 
    {
        usleep(1);
        spin--;
    }

    if (spin==0) return false;
    return true; 
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/* FIS UTILITIES*/
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void fis_reg_h2d_setup(fis_reg_h2d_t* cmd_fis, u8 command, u64 lba, u32 count, bool lba48)
{
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->cmd_ctl = 1;
    cmd_fis->command = command;

    cmd_fis->lba0 = (u8)(lba>>0);
    cmd_fis->lba1 = (u8)(lba>>8);
    cmd_fis->lba2 = (u8)(lba>>16);
    cmd_fis->device = (1<<6);

    if (lba48)
    {
        cmd_fis->lba3 = (u8)(lba>>24);
        cmd_fis->lba4 = (u8)(lba>>32);
        cmd_fis->lba5 = (u8)(lba>>40);
    }
    else cmd_fis->device |= ((lba>>24)&0xF);

    cmd_fis->count_lo = (u8)(count);
    cmd_fis->count_hi = (u8)(count>>8);
}