#include "ahci_info.h"
#include "ahci_utils.h"
#include "ahci_info.h"
#include <stdbool.h>
#include "ahci_interrupts.h"
#include "../../timer/timer.h"
#include "../../stdio.h"

ahci_handler_t ahci_handler_table[32];

void ahci_register_handler(int inum, ahci_handler_t handler)
{
    if (inum>=32) return;
    ahci_handler_table[inum] = handler;
}

void ahci_int_handling(ahci_port_t* port)
{
    port->status = true;
    // Interrupt Handling
    u32 is = port->hba_port->interrupt_status;
    for (u32 i=0;i<32;i++)
    {
        if (!(is & (1<<i))) continue;
        if (ahci_handler_table[i]) ahci_handler_table[i](port);
        else
        {
            kprintf("AHCI: Unknown interrupt no. %u\n", i);
            port->status = false;
        }
        port->hba_port->interrupt_status &= ~(1<<i);
        return;
    }
    
    // Cmd Handling
    for (int i = 0; i < max_cmd_slot_cnt; i++) {
        if (!((port->hba_port->cmd_issue|port->hba_port->sata_active) & (1<<i)))
        {
            port->cmd_done[i]=true;
        }
    }
}

void ahci_interrupt_handler(registers_t* regs)
{
    kprintf("Oh yeah\n");
    for (int i=0;i<current_port_count;i++)
    {
        bool port_int = (abar->interrupt_status >> i)&1;
        if (!port_int) continue;
        ahci_int_handling(&ports[i]);
    }
    abar->interrupt_status = -1;
    // pic_driver->write(DRIVER_CMD, PIC_DRV_CMD_SEND_EOI);
    // pic_driver->write(DRIVER_DATA, interrupt_line);
}


void ahci_handle_tfes(ahci_port_t* port)
{
    kprintf("\n");
    kprintf("AHCI: Task File Error!\n");
    for (int i=0;i<max_cmd_slot_cnt;i++)
    {
        if (port->hba_port->cmd_issue & (1<<i)) 
        {
            kprintf("Command issued: %d\n", i);
            break;
        }
    }
    u8 cmd_active = (port->hba_port->command_status >> 8)&0x1F;    
    kprintf("Command active: %u\n", cmd_active);

    ahci_stop_cmd(port->hba_port,true);
    if (port->hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ))
    {
        kprintf("AHCI: Resetting device...\n");
        ahci_port_reset(port->hba_port);
    }

    ahci_start_cmd(port->hba_port,true);
    port->status = false;
}