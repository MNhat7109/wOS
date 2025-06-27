#include "ahci_utils.h"
#include "ahci_info.h"
#include "../../timer/timer.h"
#include "../../stdio.h"
#include "../../string/string.h"

#define AHCI_SIG_ATA   0x00000101
#define AHCI_SIG_ATAPI 0xEB140101
#define AHCI_SIG_SEMB  0xC33C0101
#define AHCI_SIG_PM    0x96690101

#define AHCI_PORT_NONE   0
#define AHCI_PORT_SATA   1
#define AHCI_PORT_SATAPI 2
#define AHCI_PORT_SEMB   3
#define AHCI_PORT_PM     4

int ahci_find_avl_cmd_slots(ahci_port_t* port)
{
    u32 slots = (port->hba_port->sata_active | port->hba_port->cmd_issue);
    for (int i=0;i<max_cmd_slot_cnt;i++)
    {
        if (!(slots & 1)) 
        {
            return i;
        }
        slots>>=1;
    }
    return -1;
}

void ahci_bios_os_handoff()
{
    abar->bios_os_handoff_ctl_sts |= HBA_BOHC_OOS;
    while (1)
    {
        if (!(abar->bios_os_handoff_ctl_sts & HBA_BOHC_BOS))
            break;
    }

    int spin=0;
    while (spin < 2000 && (abar->bios_os_handoff_ctl_sts & HBA_BOHC_BB)) spin++;

}

static int ahci_check_port_type(hba_port_t* port)
{
    u32 status = port->sata_status;
    u8 det = status & 0xF;
    u8 ipm = (status >> 8) & 0xF;

    // IPM field = 0x1 -> Device is Active.
    // DET field = 0x3 -> Device and its physical layer (PHY) communication is present.
    // In order for an AHCI devices to be attached, both of these conditions must be met.
    if (det != 0x3 || ipm != 0x1) return AHCI_PORT_NONE;

    // The device is attached, get the port type from its signature
    u32 signature = port->signature;
    kprintf("%x\n", signature);
    switch (signature)
    {
    case AHCI_SIG_ATAPI: return AHCI_PORT_SATAPI;
    case AHCI_SIG_PM: return AHCI_PORT_PM;
    case AHCI_SIG_SEMB: return AHCI_PORT_SEMB;
    default: return AHCI_PORT_SATA;
    }
}

void ahci_probe_port()
{
    u32 port_impl = abar->port_implemented;
    if (current_port_count == AHCI_MAX_PORT_ENTRIES) return;
    for (u32 i=0;i<max_port_cnt;i++)
    {
        if (!(port_impl & 1)) continue;
        int port_type = ahci_check_port_type(&abar->ports[i]);
        switch (port_type)
        {
        case AHCI_PORT_SATA:
            kprintf("AHCI: SATA drive detected\n");
            ports[current_port_count].used = true;
            ports[current_port_count].num = i;
            ports[current_port_count].type = port_type;
            ports[current_port_count].hba_port = &abar->ports[i];
            memset(ports[current_port_count].cmd_done, true, sizeof(ports[current_port_count].cmd_done));
            current_port_count++;
            break;
        case AHCI_PORT_SATAPI:
            kprintf("AHCI: SATAPI drive detected\n");
            ports[current_port_count].used = true;
            ports[current_port_count].num = i;
            ports[current_port_count].type = port_type;
            ports[current_port_count].hba_port = &abar->ports[i];
            memset(ports[current_port_count].cmd_done, true, sizeof(ports[current_port_count].cmd_done));
            current_port_count++;
            break;
        case AHCI_PORT_SEMB:
            kprintf("AHCI: SEMB drive detected\n");
            // TODO: Will implement this later: probe_bridge()
            break;
        case AHCI_PORT_PM:
            kprintf("AHCI: PM drive detected\n");
            // TODO: Same, Will implement this later: probe_pm()
            break;
        default:
            kprintf("AHCI: No drive found\n");
            break;
        }
        port_impl >>= 1;
    }
}

void ahci_hba_reset()
{
    abar->global_host_ctl |= HBA_GHC_HR;
    int spin = 1000;
    while (spin>0)
    {
        if (!(abar->global_host_ctl & HBA_GHC_HR)) break;
        sleep(1);
        spin--;
    }

    // if (spin==0)
    // {
        //     kprintf("Not cleared\n");
    // }
    u32 pi = abar->port_implemented;
    for (int i=0;i<max_cmd_slot_cnt;i++)
    {
        ahci_port_shutdown(&abar->ports[i]);
        if (!(pi&1)) continue;
        ahci_port_startup(&abar->ports[i]);
        pi>>=1;
    }
}

void ahci_port_reset(hba_port_t* port)
{
    ahci_stop_cmd(port, false);
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
    ahci_start_cmd(port, false);
}

void ahci_soft_reset(ahci_port_t* ahci_port)
{
    hba_port_t* port = ahci_port->hba_port;

    int spin = 1000;
    while (spin>0&&(port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)))
    {
        sleep(1);
        spin--;
    }

    if (port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) ahci_port_reset(port);

    int slot = ahci_find_avl_cmd_slots(ahci_port);
    if (slot == -1)
    {
        kprintf("AHCI: Out of command slots. Device is busy\n");
        ahci_port_reset(port);
    }

    hba_cmd_hdr_t* cmd_hdr0 = (hba_cmd_hdr_t*)port->cmd_list_base_lo;
    cmd_hdr0 += slot;
    cmd_hdr0->cmd_fis_length = sizeof(fis_reg_h2d_t)/sizeof(u32);
    cmd_hdr0->reset = 1;
    cmd_hdr0->clear_busy = 1;

    hba_cmd_table_t* cmd_table0 = (hba_cmd_table_t*)cmd_hdr0->cmd_table_base_lo;
    memset(cmd_table0, 0, sizeof(hba_cmd_table_t));

    fis_reg_h2d_t* cmd_fis0 = (fis_reg_h2d_t*)&cmd_table0->cmd_fis;
    cmd_fis0->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis0->cmd_ctl = 0;
    cmd_fis0->control = (1<<2); // SRST

    port->cmd_issue |= (1<<slot);
    sleep(10);

    hba_cmd_hdr_t* cmd_hdr1 = (hba_cmd_hdr_t*)port->cmd_list_base_lo;
    cmd_hdr1 += slot;
    cmd_hdr1->cmd_fis_length = sizeof(fis_reg_h2d_t)/sizeof(u32);
    cmd_hdr1->reset = 0;
    cmd_hdr1->clear_busy = 0;

    hba_cmd_table_t* cmd_table1 = (hba_cmd_table_t*)cmd_hdr1->cmd_table_base_lo;
    memset(cmd_table1, 0, sizeof(hba_cmd_table_t));

    fis_reg_h2d_t* cmd_fis1 = (fis_reg_h2d_t*)&cmd_table1->cmd_fis;
    cmd_fis1->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis1->cmd_ctl = 0;
    cmd_fis1->control = 0;
    port->cmd_issue |= (1<<slot);
    sleep(10);
}

void ahci_stop_cmd(hba_port_t* port, bool reset)
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
    {
        if (reset) ahci_port_reset(port);
        //
    }
}
void ahci_stop_fis(hba_port_t* port, bool reset)
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
    {
        if (reset) ahci_port_reset(port);
        //
    }
}
void ahci_port_shutdown(hba_port_t* port)
{
    ahci_stop_cmd(port, true);
    ahci_stop_fis(port, true);
}

void ahci_start_cmd(hba_port_t* port, bool reset)
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
    
    if (spin ==0) 
    {
        if (reset) ahci_port_reset(port);
        //
    }
    port->command_status |= (1<<0);
}

void ahci_start_fis(hba_port_t* port, bool reset)
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
    
    if (spin ==0) 
    {
        if (reset) ahci_port_reset(port);
        //
    }
    port->command_status |= (1<<4);
}

void ahci_port_startup(hba_port_t* port)
{
    ahci_start_fis(port, true);
    ahci_start_cmd(port, true);
}

bool ahci_get_port_attributes(ahci_port_t* port, void* out_buffer)
{
    __asm__ volatile("cli");
    hba_port_t* hba_port = port->hba_port;
    int spin =0;

    hba_port->interrupt_enable = 0xFFFFFFFF;       // Enable PxIE (per port)
    abar->global_host_ctl |= (1 << 1);                         // Set GHC.IE (global)
    hba_port->interrupt_status = (u32)-1;
    abar->interrupt_status = (1<<port->num);

    int slot = ahci_find_avl_cmd_slots(port);
    if (slot == -1)
    {
        kprintf("AHCI: Out of command slots. Device is busy\n");
        return false;
    }    

    hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)hba_port->cmd_list_base_lo;
    cmd_hdr += slot;
    cmd_hdr->cmd_fis_length = sizeof(fis_reg_h2d_t)/sizeof(u32);
    cmd_hdr->write = 0;
    cmd_hdr->prdt_length = 1;

    
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
    memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));
    
    cmd_table->prdt_entries[0].data_base_addr_lo = (u32)out_buffer;
    cmd_table->prdt_entries[0].data_byte_cnt = 511;
    cmd_table->prdt_entries[0].interrupt = 1;
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)&cmd_table->cmd_fis;
    
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->cmd_ctl = 1;
    cmd_fis->command = ATA_CMD_IDENTIFY;

    while ((hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
        spin++;
    
    if (spin == 1000000)
    {
        kprintf("AHCI: Port is hung!\n");
        return false;
    }

    __asm__ volatile("sti");

    hba_port->cmd_issue = (1<<slot);

    while (hba_port->cmd_issue & (1<<slot))
    {
        kprintf("PxCI: addr:%x, %x\n", &hba_port->cmd_issue, hba_port->cmd_issue);
        sleep(1);
    }
    kprintf("Cmd issue cleared\n");
    // while (!(hba_port->interrupt_status & HBA_PxIS_DHRS))
    // {
    //     kprintf("PxIS: addr:%x, %x\n", &hba_port->interrupt_status, hba_port->interrupt_status);
    //     sleep(1);
    // }
    // kprintf("Cmd completed somehow without an interrupt...\n");
    // hba_port->interrupt_status = -1;
    return true;
}
