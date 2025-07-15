#include "ahci_utils.h"
#include "ahci_ports.h"
#include "ahci_hba.h"
#include "ahci_fis.h"
#include "ahci_general.h"
#include "ahci.h"

#include "../../stdint.h"
#include "../../ktime/ktime.h"
#include "../../stdio.h"
#include "../../string/string.h"

int ahci_find_avl_cmd_slots(ahci_device_entry_t* port)
{
    u32 slots = port->cmd_done;
    for (int i=0;i<port->max_cmd_slot_count;i++)
    {
        if ((slots & 1)) 
        {
            return i;
        }
        slots>>=1;
    }
    return -1;
}

int ahci_identify(ahci_driver_t* self, ahci_device_entry_t* port, void* out_buffer)
{
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    hba_port->interrupt_status = (u32)-1;
    u8 port_num = ahci_port_get_number(port);
    self->__abar->interrupt_status = (1<<(port_num));

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
        return AHCI_CMD_BUSY;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32), 1, 0);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));
    hba_setup_prdt(cmd_table, 0, out_buffer, 512);
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    fis_reg_h2d_setup(cmd_fis, ATA_CMD_IDENTIFY, 0, 0, false);

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;

        if (spin == 1000000)
    {
        return AHCI_CMD_HUNG;
    }

    hba_port->cmd_issue = (1<<slot);

    while (!(port->cmd_done & (1<<slot)))
    {
        kprintf("AHCI: Still waiting for completion...\n");
        sleep(10);
    }

    return port->general_info&AHCI_DEV_CMD_OK;
}

int ahci_read_sectors_dma_ext(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    hba_port->interrupt_status = (u32)-1;
    u8 port_num = ahci_port_get_number(port);
    self->__abar->interrupt_status = (1<<(port_num));

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
        return AHCI_CMD_BUSY;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    u16 prdtl = ((count-1)>>4)+1;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32), prdtl, 0);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

    u32 buffer_addr = (u32)buffer;
    int i=0;
    for (;i<prdtl-1;i++)
    {
        hba_setup_prdt(cmd_table, i, (void*)buffer_addr, 8192);
        buffer_addr+=8192;
        count-=16;
    }
    hba_setup_prdt(cmd_table, i, (void*)buffer_addr, (count<<9));
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    u8 cmd = ATA_CMD_READ_DMA_EXT;
    fis_reg_h2d_setup(cmd_fis, cmd, lba, count, true);

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;
    
    if (spin == 1000000)
    {
        return AHCI_CMD_HUNG;
    }

    hba_port->cmd_issue = (1<<slot);

    while (!(port->cmd_done & (1<<slot)))
    {
        kprintf("AHCI: Still waiting for completion...\n");
        sleep(10);
    }

    return port->general_info&AHCI_DEV_CMD_OK;
}

int ahci_read_sectors_dma(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    hba_port->interrupt_status = (u32)-1;
    u8 port_num = ahci_port_get_number(port);
    self->__abar->interrupt_status = (1<<(port_num));

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
        return AHCI_CMD_BUSY;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    u16 prdtl = ((count-1)>>4)+1;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32), prdtl, 0);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

    u32 buffer_addr = (u32)buffer;
    int i=0;
    for (;i<prdtl-1;i++)
    {
        hba_setup_prdt(cmd_table, i, (void*)buffer_addr, 8192);
        buffer_addr+=8192;
        count-=16;
    }
    hba_setup_prdt(cmd_table, i, (void*)buffer_addr, (count<<9));
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    u8 cmd = ATA_CMD_READ_DMA;
    fis_reg_h2d_setup(cmd_fis, cmd, lba, count, false);

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;
    
    if (spin == 1000000)
    {
        return AHCI_CMD_HUNG;
    }

    hba_port->cmd_issue = (1<<slot);

    while (!(port->cmd_done & (1<<slot)))
    {
        kprintf("AHCI: Still waiting for completion...\n");
        sleep(10);
    }

    return port->general_info&AHCI_DEV_CMD_OK;
}

int ahci_read_sectors(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    if (!(port->general_info & AHCI_DEV_ADDRMODE)) return AHCI_CMD_UNSUPPORTED;
    u64 lba_dest = lba+count;
    if (lba > port->max_lba_48 || lba_dest >port->max_lba_48)
        return AHCI_CMD_FAILURE;
    if (lba > port->max_lba_28 || lba_dest > port->max_lba_28)
    {
        if (!(port->general_info & AHCI_DEV_LBA48)) return AHCI_CMD_UNSUPPORTED;
        return ahci_read_sectors_dma_ext(self, port, lba, count, buffer);
    }
    return ahci_read_sectors_dma(self, port, lba, count, buffer);
}

/*
    AHCI WRITE SECTORS
*/

int ahci_write_sectors_dma_ext(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    hba_port->interrupt_status = (u32)-1;
    u8 port_num = ahci_port_get_number(port);
    self->__abar->interrupt_status = (1<<(port_num));

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
        return AHCI_CMD_BUSY;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    u16 prdtl = ((count-1)>>4)+1;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32), prdtl, 1);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

    u32 buffer_addr = (u32)buffer;
    int i=0;
    for (;i<prdtl-1;i++)
    {
        hba_setup_prdt(cmd_table, i, (void*)buffer_addr, 8192);
        buffer_addr+=8192;
        count-=16;
    }
    hba_setup_prdt(cmd_table, i, (void*)buffer_addr, (count<<9));
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    u8 cmd = ATA_CMD_WRITE_DMA_EXT;
    fis_reg_h2d_setup(cmd_fis, cmd, lba, count, true);

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;
    
    if (spin == 1000000)
    {
        return AHCI_CMD_HUNG;
    }

    hba_port->cmd_issue = (1<<slot);

    while (!(port->cmd_done & (1<<slot)))
    {
        kprintf("AHCI: Still waiting for completion...\n");
        sleep(10);
    }

    return port->general_info&AHCI_DEV_CMD_OK;
}

int ahci_write_sectors_dma(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    hba_port->interrupt_status = (u32)-1;
    u8 port_num = ahci_port_get_number(port);
    self->__abar->interrupt_status = (1<<(port_num));

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
        return AHCI_CMD_BUSY;

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    u16 prdtl = ((count-1)>>4)+1;
    hba_setup_cmd_hdr(cmd_hdr, slot, sizeof(fis_reg_h2d_t)/sizeof(u32), prdtl, 1);
    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    /* In the struct hba_cmd_table_t, there has already been 1 entry of PRDT, so only zero out (prdt_length-1) entries
    after the cmd_table. */
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

    u32 buffer_addr = (u32)buffer;
    int i=0;
    for (;i<prdtl-1;i++)
    {
        hba_setup_prdt(cmd_table, i, (void*)buffer_addr, 8192);
        buffer_addr+=8192;
        count-=16;
    }
    hba_setup_prdt(cmd_table, i, (void*)buffer_addr, (count<<9));
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    u8 cmd = ATA_CMD_WRITE_DMA;
    fis_reg_h2d_setup(cmd_fis, cmd, lba, count, false);

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;
    
    if (spin == 1000000)
    {
        return AHCI_CMD_HUNG;
    }

    hba_port->cmd_issue = (1<<slot);

    while (!(port->cmd_done & (1<<slot)))
    {
        kprintf("AHCI: Still waiting for completion...\n");
        sleep(10);
    }

    return port->general_info&AHCI_DEV_CMD_OK;
}

int ahci_write_sectors(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer)
{
    if (!(port->general_info & AHCI_DEV_ADDRMODE)) return AHCI_CMD_UNSUPPORTED;
    u64 lba_dest = lba+count;
    if (lba > port->max_lba_48 || lba_dest >port->max_lba_48)
        return AHCI_CMD_FAILURE;
    if (lba > port->max_lba_28 || lba_dest > port->max_lba_28)
    {
        if (!(port->general_info & AHCI_DEV_LBA48)) return AHCI_CMD_UNSUPPORTED;
        return ahci_write_sectors_dma_ext(self, port, lba, count, buffer);
    }
    return ahci_write_sectors_dma(self, port, lba, count, buffer);
}