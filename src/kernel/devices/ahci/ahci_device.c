#include "ahci_device.h"
#include "ahci_defs.h"
#include "ahci_hw.h"
#include "ahci_shared.h"

#include "../driver.h"
#include "../../stdio.h"
#include "../../time.h"
#include "../../string/string.h"
#include "../../paging/paging.h"

static bool ahci_device_alloc(u32* base, u32 size);
static bool ahci_device_free(u32 base);
static int ahci_device_check_type(hba_port_t* port);

void ahci_device_detect(struct generic_driver_t* driver, ahci_controller_t* self)
{
    u32 hcap = self->abar->host_capability;
    u32 hpi = self->abar->port_implemented;
    
    // Get some thresholds
    u32 max_port_count = (hcap>>0)&0x1F;
    u32 max_cmd_slot_count = (hcap>>8)&0x1F;
    
    for (u32 i=0;i<max_port_count;i++)
    {
        // If the port bit is not set, that means there's
        // no device connected to the port whatsoever
        if (!(hpi & (1<<i))) continue;
        
        driver_log_state(driver, DRIVER_LOG_NOTICE, "Port attributes:");
        kprintf("    ID: %u\n", i);
        int port_type = ahci_device_check_type(&self->abar->ports[i]);
        kprintf("    Type: %s\n",
            (const char*[]){"Unknown", "SATA", "SATAPI", "Bridge", "Multiplier"}
            [port_type]
        );
        
        switch (port_type)
        {
        case AHCI_PORT_TYPE_SATA:
        case AHCI_PORT_TYPE_SATAPI:
            ahci_device_set_flag(&self->devices[self->device_count], AHCI_DEV_CMD_OK);
            ahci_device_set_flag(&self->devices[self->device_count], AHCI_DEV_CONNECTED);
            ahci_device_set_id(&self->devices[self->device_count], i);
            ahci_device_set_type(&self->devices[self->device_count], port_type);

            self->devices[self->device_count].cmd_done = -1;
            self->devices[self->device_count].cmd_ok = -1;
            self->devices[self->device_count].hba_port = &self->abar->ports[i];
            self->devices[self->device_count].max_cmd_slot_count
            = max_cmd_slot_count;
            self->device_count++;
            break;
        default:
            break;
        }
    }

    /* TODO: Add support for SEMB, and PM. 
    Since they are bridges and multipliers, there should be a function 
    to recursively find the end-device 
    and put it in the container.*/
}

void ahci_device_reset_all(struct generic_driver_t* driver, ahci_controller_t* self)
{
    u32 hpi = self->abar->port_implemented;
    u32 i=0;

    while (hpi)
    {
        if (!hba_port_shutdown(&self->abar->ports[i]))
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Device is not ready. Skipping...");
            continue;
        }

        if (!hba_port_startup(&self->abar->ports[i]))
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Failed to start the device. Skipping...");
            continue;
        }
        hpi>>=1;
        i++;
    }
}

bool ahci_device_rebase(struct generic_driver_t* driver, ahci_dev_t* device)
{
    if (!ahci_device_shutdown(device))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Device is not ready");
        return false;
    }

    // Command list setup
    if (!ahci_device_alloc(&device->hba_port->cmd_list_base_lo, 1024))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Out of physical memory for command list");
        return false;
    }

    // Command header setup
    hba_cmd_hdr_t* chba = (hba_cmd_hdr_t*)(device->hba_port->cmd_list_base_lo);
    for (int i=0;i<device->max_cmd_slot_count;i++)
    {
        chba[i].prdt_length = 8;

        if (!ahci_device_alloc(&chba[i].cmd_table_base_lo, 4096))
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Out of physical memory for command table");
            return false;
        }
    }

    if (!ahci_device_startup(device))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Failed to start the device");
        return false;
    }

    return true;
}

bool ahci_device_nuke(struct generic_driver_t* driver, ahci_dev_t* device)
{
    // If the device has been disconnected, return true right away
    if (!(device->general_info & AHCI_DEV_CONNECTED)) return true;
    
    // Shut down the device
    if (!ahci_device_shutdown(device))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Device is not ready");
        return false;
    }

    // Free the command table
    hba_cmd_hdr_t* chba = (hba_cmd_hdr_t*)(device->hba_port->cmd_list_base_lo);
    for (int i=0;i<device->max_cmd_slot_count;i++)
    {
        chba[i].prdt_length = 0;

        if (!ahci_device_free(chba[i].cmd_table_base_lo))
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Failed to deallocate the command table");
            return false;
        }
        chba[i].cmd_table_base_lo = NULL;
    }

    // Free FIS base
    if (!ahci_device_free(device->hba_port->fis_base_lo))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Failed to deallocate FIS");
        return false;
    }
    device->hba_port->fis_base_lo=NULL;

    // Free the command list
    if (!ahci_device_free(device->hba_port->cmd_list_base_lo))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Failed to deallocate the command list");
        return false;
    }
    device->hba_port->cmd_list_base_lo=NULL;

    device->cmd_done=0; // Wipe everything out
    ahci_device_clear_flag(device, AHCI_DEV_CONNECTED);

    return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
/*  GLOBAL HELPER FUNCTIONS */
/*  to make our lives better */
//////////////////////////////////////////////////
//////////////////////////////////////////////////

u8 ahci_device_get_id(ahci_dev_t* device)
{
    return (device->general_info >> 8) & 0xFF;
}

u8 ahci_device_get_type(ahci_dev_t* device)
{
    return (device->general_info >> 4) & 0xF;
}

void ahci_device_set_flag(ahci_dev_t* device, u32 flag)
{
    device->general_info |= flag;
}

void ahci_device_clear_flag(ahci_dev_t* device, u32 flag)
{
    device->general_info &= ~flag;
}

void ahci_device_set_id(ahci_dev_t* device, u8 number)
{
    device->general_info |= (number << 8);
}

void ahci_device_set_type(ahci_dev_t* device, u8 type)
{
    device->general_info |= (type << 4);
}

bool ahci_device_startup(ahci_dev_t* device)
{
    return hba_port_startup(device->hba_port);
}

bool ahci_device_shutdown(ahci_dev_t* device)
{
    return hba_port_shutdown(device->hba_port);
}

bool ahci_device_reset(ahci_dev_t* device)
{
    for (int i=0;i<3;i++)
    {
        if (hba_port_reset(device->hba_port)) return true;
    }
    return false;
}

bool ahci_device_wait_complete(ahci_dev_t* device, int slot)
{
    int spin=0;
    while (!(device->cmd_done & (1<<slot)) && spin < 100)
    {
        usleep(10);
        spin++;
    }
    if (spin==100) return false;
    return true;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

static bool ahci_device_alloc(u32* base, u32 size)
{
    if (size > 4096) return false;
    u32 raw_base = page_alloc_request();
    if (!raw_base) return false;
    page_manager_map_memory(raw_base,raw_base);

    memset((void*)raw_base, 0, size);
    *base = raw_base;
    return true;
}

static bool ahci_device_free(u32 base)
{
    u32 phys_base = page_manager_unmap_memory(base);
    if (!phys_base) return false;

    page_alloc_free(phys_base);
    return true;
}

static int ahci_device_check_type(hba_port_t* port)
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

