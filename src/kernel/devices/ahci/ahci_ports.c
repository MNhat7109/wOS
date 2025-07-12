#include "ahci_ports.h"
#include "../../stdio.h"
#include "../../paging/paging.h"
#include "../../string/string.h"

u8 ahci_port_get_number(ahci_device_entry_t* device)
{
    return (device->general_info >> 8) & 0xFF;
}

u8 ahci_port_get_type(ahci_device_entry_t* device)
{
    return (device->general_info >> 4) & 0xF;
}

void ahci_port_set_flag(ahci_device_entry_t* device, u32 flag)
{
    device->general_info |= flag;
}

void ahci_port_clear_flag(ahci_device_entry_t* device, u32 flag)
{
    device->general_info &= ~flag;
}

void ahci_port_set_number(ahci_device_entry_t* device, u8 number)
{
    device->general_info |= (number << 8);
}

void ahci_port_set_type(ahci_device_entry_t* device, u8 type)
{
    device->general_info |= (type << 4);
}

static bool ahci_allocate_base(u32* base, u32 size)
{
    if (size > 4096) return false;
    u32 raw_base = page_alloc_request();
    if (!raw_base) return false;
    page_manager_map_memory(raw_base,raw_base);

    memset((void*)raw_base, 0, size);
    *base = raw_base;
    return true;
}

static int ahci_port_type_check(hba_port_t* port)
{
    u32 port_sig = hba_port_type_detect(port);
    switch (port_sig)
    {
    case AHCI_SIG_ATA: return AHCI_PORT_TYPE_SATA;
    case AHCI_SIG_ATAPI: return AHCI_PORT_TYPE_SATAPI;
    case AHCI_SIG_SEMB: return AHCI_PORT_TYPE_SEMB;
    case AHCI_SIG_PM: return AHCI_PORT_TYPE_PM;
    default:
        return 0;
    }
}

void ahci_port_setup(ahci_ports_t* self, hba_memory_t* abar)
{
    u32 cap = abar->host_capability;
    if (self->current_device_count == AHCI_MAX_PORT_ENTRIES) return;
    self->max_port_count = (cap>>0)&0x1F;
    self->current_device_count = 0;
}

void ahci_port_detect(ahci_ports_t* self, hba_memory_t* abar)
{
    u32 port_impl = abar->port_implemented;
    u32 cap = abar->host_capability;
    u32 max_cmd_slot_count = (cap>>8)&0x1F;

    for (u32 i=0;i<self->max_port_count;i++)
    {
        if (!(port_impl & (1<<i))) continue;
        kprintf("AHCI: At port %u: ", i);
        int port_type = ahci_port_type_check(&abar->ports[i]);
        kprintf("%s drive found\n"
            , (const char*[]){"No", "SATA", "SATAPI", "SEMB", "PM"}[port_type]);
        
        if (port_type == AHCI_PORT_TYPE_SATA || port_type == AHCI_PORT_TYPE_SATAPI)
        {
            ahci_port_set_flag(&self->devices[self->current_device_count], AHCI_DEV_CMD_OK);
            ahci_port_set_flag(&self->devices[self->current_device_count], AHCI_DEV_CONNECTED);
            ahci_port_set_number(&self->devices[self->current_device_count], i);
            ahci_port_set_type(&self->devices[self->current_device_count], port_type);
            
            self->devices[self->current_device_count].cmd_done = -1;
            self->devices[self->current_device_count].hba_port = &abar->ports[i];
            self->devices[self->current_device_count].max_cmd_slot_count
            = max_cmd_slot_count;
            self->current_device_count++;
        }

        /* TODO: Add support for SEMB, and PM. 
        Since they are bridges and multipliers, there should be a function 
        to recursively find the end-device 
        and put it in the container.*/
    }
}

bool ahci_port_startup_dev(ahci_device_entry_t* device)
{
    return hba_port_startup(device->hba_port);
}

bool ahci_port_shutdown_dev(ahci_device_entry_t* device)
{
    return hba_port_shutdown(device->hba_port);
}

bool ahci_port_reset_dev(ahci_device_entry_t* device)
{
    for (int i=0;i<3;i++)
    {
        if (hba_port_reset(device->hba_port)) return true;
    }
    return false;
}

bool ahci_port_rebase_dev(ahci_device_entry_t* device)
{
    if (!ahci_port_shutdown_dev(device))
    {
        kprintf("AHCI: Device at port is not ready\n");
        return false;
    }

    // Command list setup
    if (!ahci_allocate_base(&device->hba_port->cmd_list_base_lo, 1024))
    {
        kprintf("AHCI: Out of physical memory for command list!\n");
        return false;
    }

    // FIS setup
    if (!ahci_allocate_base(&device->hba_port->fis_base_lo, 256))
    {
        kprintf("AHCI: Out of physical memory for FIS!\n");
        return false;
    }

    // Command header setup
    hba_cmd_hdr_t* chba = (hba_cmd_hdr_t*)(device->hba_port->cmd_list_base_lo);
    for (int i=0;i<device->max_cmd_slot_count;i++)
    {
        chba[i].prdt_length = 8;

        if (!ahci_allocate_base(&chba[i].cmd_table_base_lo, 4096))
        {
            kprintf("AHCI: Out of physical memory for command table!\n");
            return false;
        }
    }

    if (!ahci_port_startup_dev(device))
    {
        kprintf("AHCI: Failed to start device\n");
        return false;
    }

    return true;
}

bool ahci_port_nuke_dev(ahci_device_entry_t* device)
{
    if (!ahci_port_shutdown_dev(device))
    {
        kprintf("AHCI: Device at port is not ready\n");
        return false;
    }

    hba_cmd_hdr_t* chba = (hba_cmd_hdr_t*)(device->hba_port->cmd_list_base_lo);
    for (int i=0;i<device->max_cmd_slot_count;i++)
    {
        chba[i].prdt_length = 0;

        if (!page_manager_unmap_memory(chba[i].cmd_table_base_lo))
        {
            kprintf("AHCI: Failed to deallocate memory for command table!\n");
            return false;
        }
    }

    if (!page_manager_unmap_memory(device->hba_port->fis_base_lo))
    {
        kprintf("AHCI: Failed to deallocate memory for FIS!\n");
        return false;
    }

    if (!page_manager_unmap_memory(device->hba_port->cmd_list_base_lo))
    {
        kprintf("AHCI: Failed to deallocate memory for command list!\n");
        return false;
    }

    ahci_port_clear_flag(device, AHCI_DEV_CONNECTED);
    return true;
}

void ahci_port_reset_all(ahci_ports_t* self, hba_memory_t* abar)
{
    u32 pi = abar->port_implemented;
    for (u32 i=0;i<self->max_port_count;i++)
    {
        hba_port_shutdown(&abar->ports[i]);
        if (!(pi & (1<<i))) continue;
        hba_port_startup(&abar->ports[i]);
    }
}