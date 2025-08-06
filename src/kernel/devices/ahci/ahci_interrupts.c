#include "ahci_interrupts.h"
#include "ahci_hw.h"
#include "ahci_defs.h"
#include "ahci_device.h"
#include "ahci_hw.h"
#include "ahci_shared.h"
#include "ahci_utils.h"

#include "ahci_irq.h"

#include "../../x86/x86.h"
#include "../../stdio.h"
#include "../driver.h"
#include "../../hal/interrupt/isr.h"

typedef void (*ahci_dev_int_handler_t)(ahci_dev_t* dev, u8 slot);
static struct
{
    struct generic_driver_t* driver;
    ahci_dev_int_handler_t ahci_dev_handler_table[AHCI_CONFIG_MAX_CMD_SLOT];
    bool per_port_int;
    bool per_ctl_int;
} ahci_int_data;

static void ahci_interrupt_end(u32 int_num);
static void ahci_load_handler(u32 int_bit, ahci_dev_int_handler_t handler);
static void ahci_general_ctl_interrupt_handler(registers_t* regs, void* ctx);
static void ahci_general_port_interrupt_handler(ahci_dev_t* device);

static void ahci_handle_dhrs(ahci_dev_t* dev, u8 slot);
static void ahci_handle_pss(ahci_dev_t* dev, u8 slot);
static void ahci_handle_dss(ahci_dev_t* dev, u8 slot);
static void ahci_handle_tfes(ahci_dev_t* dev, u8 slot);

void ahci_interrupt_setup(struct generic_driver_t* driver, ahci_controller_t* controller)
{
    ahci_int_data.driver = driver;

    _x86_disable_interrupt();

    // Set Interrupt Enable bit
    controller->abar->global_host_ctl |= HBA_GHC_IE;

    // TODO: Add the MSI/MSI-X support
    ahci_interrupt_legacy_setup(controller, ahci_general_ctl_interrupt_handler);
    ahci_int_data.per_ctl_int = true;
    ahci_int_data.per_port_int = false;

    ahci_load_handler(0, ahci_handle_dhrs);
    ahci_load_handler(1, ahci_handle_pss);
    ahci_load_handler(2, ahci_handle_dss);
    ahci_load_handler(30, ahci_handle_tfes);

    _x86_enable_interrupt();

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Interrupt has been enabled");
}

void ahci_interrupt_disable(struct generic_driver_t* driver, ahci_controller_t* controller)
{
    _x86_disable_interrupt();

    // TODO: Add the MSI/MSI-X support
    ahci_interrupt_legacy_disable(controller);

    // Clear the Interrupt Enable bit
    controller->abar->global_host_ctl &= ~HBA_GHC_IE;

    _x86_enable_interrupt();

    driver_log_state(driver,DRIVER_LOG_NOTICE, "Interrupt has been disabled");
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

static void ahci_interrupt_end(u32 int_num)
{
    // TODO: Add MSI/MSI-X support, right now, just pass this to the legacy utils
    ahci_interrupt_legacy_end(int_num);
}

static void ahci_load_handler(u32 int_bit, ahci_dev_int_handler_t handler)
{
    if (int_bit>=AHCI_CONFIG_MAX_CMD_SLOT) return;
    ahci_int_data.ahci_dev_handler_table[int_bit] = handler;
}

static void ahci_general_ctl_interrupt_handler(registers_t* regs, void* ctx)
{
    if (!ctx)
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "Context is NULL. Skipping interrupt...");
        return;
    }
    
    ahci_controller_t* ctl = (ahci_controller_t*)ctx;

    driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "Interrupt pending at controller:");
    // kprintf("   ID: %u\n", ctl->)

    for (u32 dev_idx=0;dev_idx<ctl->device_count;dev_idx++)
    {
        ahci_dev_t* dev = &ctl->devices[dev_idx];
        // Get the real port index (HBA port index)
        u8 prt_idx = ahci_device_get_id(dev);

        // For every bit set in the IS field in the ABAR, 
        // there's a port waiting for the interrupt to be cleared

        // If the bit is cleared, there's no interrupt coming from that port
        if (!((ctl->abar->interrupt_status >> prt_idx)&1)) continue;

        ahci_general_port_interrupt_handler(dev);
        ctl->abar->interrupt_status = (1<<prt_idx);
    }

    if (ahci_int_data.per_ctl_int) ahci_interrupt_end(ctl->interrupt_num);
}

static void ahci_general_port_interrupt_handler(ahci_dev_t* device)
{
    ahci_device_set_flag(device, AHCI_DEV_CMD_OK);
    device->cmd_ok = (u32)-1; // All of the cmd slots are ok initially

    // Handle the cmd slot
    device->cmd_done = device->hba_port->cmd_issue|device->hba_port->sata_active;

    // Handles the interrupt
    u32 pis = device->hba_port->interrupt_status;
    for (u32 i=0;i<device->max_cmd_slot_count;i++)
    {
        // If the interrupt bit is cleared, there's no interrupt at that cmd slot
        if (!(pis & (1<<i))) continue;

        if (ahci_int_data.ahci_dev_handler_table[i]) 
            ahci_int_data.ahci_dev_handler_table[i](device, i);
        else
        {
            driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE,
                "Unknown interrupt:");
            kprintf("    Location: bit %u\n", i);
            device->cmd_ok &= ~(1<<i);
        }
        device->hba_port->interrupt_status=(1<<i);
        device->cmd_done |= (1<<i); // Set the "done" bit in the cmd slot
    }

    // If all of the commands do not turn out fine, clear the overall OK flag
    if (device->cmd_ok != (u32)-1) ahci_device_clear_flag(device, AHCI_DEV_CMD_OK);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/* INTERRUPT BIT HANDLERS */
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static void ahci_handle_dhrs(ahci_dev_t* dev, u8 slot)
{
    driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "DHRS handled");

    volatile u8* fis_base = (u8*)dev->hba_port->fis_base_lo;
    volatile fis_reg_d2h_t* fis_int = (fis_reg_d2h_t*)(fis_base+0x40);

    // Run a few checks first before handling the command slot

    if (fis_int->fis_type != FIS_TYPE_REG_D2H)
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "Unexpected FIS type. Skipping interrupt...");
        return;
    }

    if (!(fis_int->interrupt & (1<<7)))
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "Interrupt misfired. Skipping...");
        return;
    }

    u8 status = fis_int->status;
    u8 error = fis_int->error;

    if (status & ATA_SR_ERR)
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "DHRS: Command failed.");
        kprintf("   Error: 0x%x\n", error);
        kprintf("   Location: Device %u, Slot %u\n", ahci_device_get_id(dev), slot);
        
        dev->cmd_ok &= ~(1<<slot); // Clear the "ok" bit in the cmd slot
    }
    else
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "DHRS: Command completed.");
        ahci_device_set_flag(dev, AHCI_DEV_CMD_OK);
        dev->cmd_ok |= (1<<slot); // Set the "ok" bit in the cmd slot
    }
}

static void ahci_handle_pss(ahci_dev_t* dev, u8 slot)
{
    driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "PSS handled");
    dev->cmd_ok |= (1<<slot); // Set the "ok" bit in the cmd slot
}

static void ahci_handle_dss(ahci_dev_t* dev, u8 slot)
{
    driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "DSS handled");
    dev->cmd_ok |= (1<<slot); // Set the "ok" bit in the cmd slot
}

static void ahci_handle_tfes(ahci_dev_t* dev, u8 slot)
{
    driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "Task File Error");
    driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "Dump:");
    kprintf("   Command issued: %u\n", slot);

    u8 cmd_active = (dev->hba_port->command_status>>8)&0x1F;
    kprintf("   Command currently active: %u\n", cmd_active);

    // Retry
    int retry_count = 0;
    while ((dev->hba_port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ))
    && retry_count<3)
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_NOTICE, "Trying to reset the device...");
        if (hba_port_reset(dev->hba_port)) break;
        retry_count++;
    }

    if (retry_count==3)
    {
        driver_log_state(ahci_int_data.driver, DRIVER_LOG_WARN, "Resetting failed. The device will be disconnected");
        ahci_device_nuke(ahci_int_data.driver, dev);
    }

    dev->cmd_ok &= ~(1<<slot); // Clear the "ok" bit in the cmd slot
}