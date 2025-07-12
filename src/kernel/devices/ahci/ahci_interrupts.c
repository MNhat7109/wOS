#include "ahci_hba.h"
#include "ahci_fis.h"
#include "ahci_ports.h"
#include "ahci_general.h"
#include "ahci_interrupts.h"
#include <stdbool.h>
#include "../../hal/cpu/ioapic.h"
#include "../../hal/cpu/lapic.h"
#include "../../ktime/ktime.h"
#include "../../stdio.h"
#include "../../x86/x86.h"

typedef void (*ahci_handler_t)(ahci_device_entry_t* port);
ahci_handler_t ahci_handler_table[32];

static struct
{
    hba_memory_t* abar;
    ahci_ports_t* ports;
    u8 irq;
} ahci_interrupt;

void ahci_register_handler(int inum, ahci_handler_t handler)
{
    if (inum>=32) return;
    ahci_handler_table[inum] = handler;
}

void ahci_int_handling(ahci_device_entry_t* port)
{
    ahci_port_set_flag(port, AHCI_DEV_CMD_OK);
    // Interrupt Handling
    u32 is = port->hba_port->interrupt_status;
    for (u32 i=0;i<port->max_cmd_slot_count;i++)
    {
        if (!(is & (1<<i))) continue;
        if (ahci_handler_table[i]) ahci_handler_table[i](port);
        else
        {
            kprintf("AHCI: Unknown interrupt no. %u\n", i);
            ahci_port_clear_flag(port, AHCI_DEV_CMD_OK);
        }
    }
    port->hba_port->interrupt_status = -1;
    
    // TODO: There's like a better, O(1) time to do this.
    // Reassign the whole thing to PxCI | PxSACT
    // Shouldn't take linear time to do this stuff.
    // Cmd Handling
    for (int i = 0; i < port->max_cmd_slot_count; i++) {
        if (!((port->hba_port->cmd_issue|port->hba_port->sata_active) & (1<<i)))
        {
            port->cmd_done |= (1<<i);
        }
    }
}

void ahci_interrupt_handler(registers_t* regs)
{
    ahci_ports_t* port_container = ahci_interrupt.ports;
    hba_memory_t* abar = ahci_interrupt.abar;
    for (int i=0;i<port_container->current_device_count;i++)
    {
        bool port_int = (abar->interrupt_status >> i)&1;
        if (!port_int) continue;
        ahci_int_handling(&port_container->devices[i]);
    }
    abar->interrupt_status = -1;
    LAPIC_send_eoi();
}


void ahci_handle_tfes(ahci_device_entry_t* port)
{
    kprintf("\n");
    kprintf("AHCI: Task File Error!\n");
    for (int i=0;i<port->max_cmd_slot_count;i++)
    {
        if (port->hba_port->cmd_issue & (1<<i)) 
        {
            kprintf("Command issued: %d\n", i);
            break;
        }
    }
    u8 cmd_active = (port->hba_port->command_status >> 8)&0x1F;    
    kprintf("Command active: %u\n", cmd_active);

    hba_port_cmd_stop(port->hba_port);
    int retry_count = 0;
    while ((port->hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) 
    && retry_count < 3)
    {
        kprintf("AHCI: Resetting device...\n");
        hba_port_reset(port->hba_port);
        retry_count++;
    }

    if (retry_count == 3)
    {
        ahci_port_nuke_dev(port);
    }

    hba_port_cmd_start(port->hba_port);
    ahci_port_clear_flag(port, AHCI_DEV_CMD_OK);
}

void ahci_handle_dhrs(ahci_device_entry_t* port)
{
    kprintf("AHCI Interrupt: DHRS handled\n");

    volatile u8* fis_base = (u8*)port->hba_port->fis_base_lo;
    volatile fis_reg_d2h_t* fis_int = (fis_reg_d2h_t*)(fis_base+0x40);

    u8 status = fis_int->status;
    u8 error = fis_int->error;

    if (status & ATA_SR_ERR)
    {
        kprintf("DHRS: Command failed. Error: 0x%x\n", error);
        // read_error()
        ahci_port_clear_flag(port, AHCI_DEV_CMD_OK);
    }
    else
    {
        kprintf("DHRS: Command completed.\n");
        ahci_port_set_flag(port, AHCI_DEV_CMD_OK);
    }
}

void ahci_handle_pss(ahci_device_entry_t* port)
{
    kprintf("AHCI Interrupt: PSS handled\n");
    ahci_port_set_flag(port, AHCI_DEV_CMD_OK);
}

void ahci_handle_dss(ahci_device_entry_t* port)
{
    kprintf("AHCI Interrupt: DSS handled\n");
    ahci_port_set_flag(port, AHCI_DEV_CMD_OK);
}

void ahci_interrupt_setup(hba_memory_t* abar, ahci_ports_t* port_container, u8 irq)
{
    ahci_interrupt.abar = abar;
    ahci_interrupt.ports = port_container;
    ahci_interrupt.irq = irq;
    
    _x86_disable_interrupt();
    ahci_interrupt.abar->global_host_ctl |= (HBA_GHC_IE);
    IRQ_setup(IOAPIC_irq_to_gsi(ahci_interrupt.irq), ahci_interrupt_handler);
    ahci_register_handler(0, ahci_handle_dhrs);
    ahci_register_handler(1, ahci_handle_pss);
    ahci_register_handler(2, ahci_handle_dss);
    ahci_register_handler(30, ahci_handle_tfes);
    _x86_enable_interrupt();
}