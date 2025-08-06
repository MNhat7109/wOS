#include "ahci_utils.h"
#include "ahci_device.h"
#include "ahci_hw.h"

#include "../../stdint.h"
#include "../../stdio.h"
#include "../../time.h"
#include "../driver.h"

#define MAX_SUPPORTED_CMD 256
typedef int (*ahci_ioctl_func_t)(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx);

static int ahci_read(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx);
static int ahci_write(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx);
static int ahci_identify(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx);

ahci_ioctl_func_t ahci_utils_table[MAX_SUPPORTED_CMD] = {
    [AHCI_IOCTL_READ] = &ahci_read,
    [AHCI_IOCTL_WRITE] = &ahci_write,
    [AHCI_IOCTL_IDENTIFY] = &ahci_identify,
    [AHCI_IOCTL_PLACEHOLDER] = NULL
};

static void ahci_detect_ata(struct generic_driver_t* driver, ahci_dev_t* device, u16* identify_buffer);
static void ahci_detect_atapi(struct generic_driver_t* driver, ahci_dev_t* device, u16* identify_buffer);

bool ahci_handle_ioctl(struct generic_driver_t* driver, ahci_dev_t* dev, int op, void* ctx)
{
    bool loop_run = true; bool result;
    int retry_cnt = 0;

    while (loop_run)
    {
        int ioctl_status = ahci_ioctl(driver, dev, op, ctx);
        switch (ioctl_status)
        {
            case AHCI_CMD_SUCCESS:
                result = true;
                loop_run=false;
                break;
            case AHCI_CMD_HUNG:
                usleep(100);
                if (retry_cnt==3) 
                {
                    result=false;
                    loop_run=false;
                }
                retry_cnt++;
                break;
            default:
                result=false;
                loop_run=false;
                break;
        }
    }

    return result;
}


bool ahci_device_identify(struct generic_driver_t* driver, ahci_dev_t* device)
{
    u16 identify_buffer[256];
    bool id_status = ahci_handle_ioctl(driver, device, AHCI_IOCTL_IDENTIFY, identify_buffer);
    if (!id_status)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Cannot identify this device");
        return false;
    }   

    u8 dev_type = ahci_device_get_type(device);
    bool result=false;
    switch (dev_type)
    {
        // We either only support SATA or SATAPI
        case AHCI_PORT_TYPE_SATA:
            ahci_detect_ata(driver, device, identify_buffer);
            result = true;
            break;
        case AHCI_PORT_TYPE_SATAPI:
            ahci_detect_atapi(driver, device, identify_buffer);
            result = true;
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_WARN, "Unknown device type");
            result = false;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/* GLOBAL HELPER FUNCTIONS */
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int ahci_find_cmd_slot(ahci_dev_t* dev)
{
    for (u32 i=0;i<dev->max_cmd_slot_count;i++)
    {
        if (dev->cmd_done & (1<<i)) return i;
    }
    return -1;
}

int ahci_ioctl(struct generic_driver_t* driver, ahci_dev_t* dev, ahci_ioctl_ops_t cmd, void* ctx)
{
    if (ahci_utils_table[cmd]) return ahci_utils_table[cmd](driver, dev, ctx);
    if (cmd>AHCI_IOCTL_PLACEHOLDER)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, 
            "Out of bound");
        return AHCI_CMD_FAILURE;
    }

    driver_log_state(driver, DRIVER_LOG_WARN, 
        "A placeholder ioctl function has been called. That means this function is probably not implemented");
    return AHCI_CMD_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ahci_detect_ata(struct generic_driver_t* driver, ahci_dev_t* device, u16* identify_buffer)
{
    // Parse that identify buffer
    if (identify_buffer[49] & (1<<9)) ahci_device_set_flag(device, AHCI_DEV_ADDRMODE);
    if (identify_buffer[83] & (1<<10)) ahci_device_set_flag(device, AHCI_DEV_LBA48);
    
    if (device->general_info & AHCI_DEV_LBA48)
    {
        device->max_lba_lo = *(u32*)&identify_buffer[100];
        device->max_lba_hi = *(u32*)&identify_buffer[102];
    }
    else device->max_lba_lo = *(u32*)&identify_buffer[60];

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Device specifications:");
    kprintf("   Port num: %u, Type: %s\n", 
        ahci_device_get_id(device), (const char*[]){"", "SATA", "SATAPI", "", ""}[ahci_device_get_type(device)]);
    kprintf("   Addressing mode: %s\n", 
        (const char*[]){"CHS", "LBA"}[(device->general_info>>2)&1]);
    
    if (!(device->general_info & AHCI_DEV_ADDRMODE))
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "This device will NOT support ioctl due to lack of LBA support");
        return;
    }

    kprintf("   Additional capabilities: %s\n", 
        (const char*[]){"LBA28", "LBA48"}[(device->general_info>>3)&1]);

    u64 max_lba_disp = (device->max_lba_lo|((u64)device->max_lba_hi<<32));
    kprintf("   Maximum User Addressable Sector Count: %llu\n", max_lba_disp);
    return;
}

static void ahci_detect_atapi(struct generic_driver_t* driver, ahci_dev_t* device, u16* identify_buffer)
{
    // Parse that identify buffer
    // TODO
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* STANDARD IOCTL FUNCTIONS */
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

static int ahci_call_wrapper(struct generic_driver_t* driver, ahci_dev_t* dev, bool write, bool use_lba, u8 cmd, 
    u64 lba, u16 count, void* buffer)
{
    int slot = ahci_find_cmd_slot(dev);
    if (slot==-1)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, 
            "Device is out of command slots. Wait for a few seconds, then try again");
        return AHCI_CMD_BUSY;
    }

    bool cmd_status = ahci_prepare_cmd(
        dev->hba_port,
        slot,
        cmd, 
        count, 
        use_lba,
        write,
        lba, 
        buffer);

    if (!cmd_status)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Invalid param value, check the buffer or the sector count.");
        return AHCI_CMD_FAILURE;
    }

    bool wait = ahci_wait_tfd(dev->hba_port);
    if (!wait)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "The device is not responding");
        return AHCI_CMD_HUNG;
    }

    dev->hba_port->cmd_issue=(1<<slot);
    wait = ahci_device_wait_complete(dev, slot);
    if (!wait)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Command timed out");
        return AHCI_CMD_HUNG;
    }

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Call complete!");
    return (dev->cmd_ok & (1<<slot))?AHCI_CMD_SUCCESS:AHCI_CMD_FAILURE;
}

//////////////////////////
//// READ
//////////////////////////

static int ahci_read_sectors_dma_ext(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer);
static int ahci_read_sectors_dma(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer);

static int ahci_read(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx)
{
    // Convert the raw context to something we can digest
    ahci_ioctl_rw_t* ioctl_read = (ahci_ioctl_rw_t*)ctx;
    
    u64 actual_lba = ioctl_read->lba;
    u16 actual_cnt = ioctl_read->sector_count;

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Now reading from device:");
    kprintf("   Device ID: %u, LBA: 0x%llx, numsectors: %u\n", 
        ahci_device_get_id(dev), actual_lba, actual_cnt);
        

    // Check if the LBA addressing support bit is set
    // It's like the start of 21st century, no one uses CHS anymore.
    if (!(dev->general_info & AHCI_DEV_ADDRMODE)) 
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "This device does NOT support Logical Block Addressing mode. Skipping...");
        return AHCI_CMD_UNSUPPORTED;
    }

    u64 target_lba = actual_lba+actual_cnt-1;
    u64 lba48_max = (u64)dev->max_lba_lo | (u64)(dev->max_lba_hi<<32);
    u64 lba28_max = lba48_max==(u64)dev->max_lba_lo? lba48_max : 0xFFFFFFF;
    
    // Bound-check!
    if (actual_lba > lba48_max || target_lba >lba48_max)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "LBA out of bound. Double check the LBA and/or the sector count.");
        return AHCI_CMD_FAILURE;
    }

    // Handle LBA48 mode
    if (actual_lba>lba28_max || target_lba>lba28_max)
    {
        // Check for LBA48 support
        if (dev->general_info & AHCI_DEV_LBA48)
            return ahci_read_sectors_dma_ext(driver, dev, 
                actual_lba, 
                actual_cnt, 
                ioctl_read->io_buffer_base);

        driver_log_state(driver, DRIVER_LOG_WARN, 
            "This device does NOT support LBA48 mode. Truncating:");
        kprintf("   LBA: 0x%x => 0x%x, Count: %u", actual_lba, lba28_max-actual_cnt, actual_cnt);
        actual_lba = lba28_max-actual_cnt;
    }

    // Check if the sector count leaks out. If so, truncate to fit.
    u16 count_max = lba28_max-actual_lba+1;
    if (actual_cnt > count_max)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Sector count too large for LBA28. Truncating:");
        kprintf("   LBA: 0x%x, Count: %u => %u", actual_lba, actual_cnt, count_max);
        actual_cnt = count_max;
    }

    return ahci_read_sectors_dma(driver, dev, 
        actual_lba, 
        actual_cnt, 
        ioctl_read->io_buffer_base);
}

static int ahci_read_sectors_dma_ext(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        false, // Read => write = 0
        true, // Use the LBA param
        ATA_CMD_READ_DMA_EXT, 
        lba, 
        count, 
        buffer);
}

static int ahci_read_sectors_dma(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        false, // Read => write = 0
        true, // Use the LBA param
        ATA_CMD_READ_DMA, 
        lba, 
        count, 
        buffer);
}

//////////////////////////
//// WRITE
//////////////////////////

static int ahci_write_sectors_dma_ext(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer);
static int ahci_write_sectors_dma(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer);

static int ahci_write(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx)
{
    // Convert the raw context to something we can digest
    ahci_ioctl_rw_t* ioctl_write = (ahci_ioctl_rw_t*)ctx;

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Now writing from device:");
    kprintf("   Device ID: %u, LBA: 0x%x, numsectors: %u\n", 
        ahci_device_get_id(dev), ioctl_write->lba, ioctl_write->sector_count);

    u64 actual_lba = ioctl_write->lba;
    u16 actual_cnt = ioctl_write->sector_count;

    // Check if the LBA addressing support bit is set
    // It's like the start of 21st century, no one uses CHS anymore.
    if (!(dev->general_info & AHCI_DEV_ADDRMODE)) 
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "This device does NOT support Logical Block Addressing mode. Skipping...");
        return AHCI_CMD_UNSUPPORTED;
    }

    u64 target_lba = actual_lba+actual_cnt-1;
    u64 lba48_max = (u64)dev->max_lba_lo | (u64)(dev->max_lba_hi<<32);
    u64 lba28_max = lba48_max==(u64)dev->max_lba_lo? lba48_max : 0xFFFFFFF;
    
    // Bound-check!
    if (actual_lba > lba48_max || target_lba >lba48_max)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "LBA out of bound. Double check the LBA and/or the sector count.");
        return AHCI_CMD_FAILURE;
    }

    // Handle LBA48 mode
    if (actual_lba>lba28_max || target_lba>lba28_max)
    {
        // Check for LBA48 support
        if (dev->general_info & AHCI_DEV_LBA48)
            return ahci_write_sectors_dma_ext(driver, dev, 
                actual_lba, 
                actual_cnt, 
                ioctl_write->io_buffer_base);

        driver_log_state(driver, DRIVER_LOG_WARN, 
            "This device does NOT support LBA48 mode. Truncating:");
        kprintf("   LBA: 0x%x => 0x%x, Count: %u", actual_lba, lba28_max-actual_cnt, actual_cnt);
        actual_lba = lba28_max-actual_cnt;
    }

    // Check if the sector count leaks out. If so, truncate to fit.
    u16 count_max = lba28_max-actual_lba+1;
    if (actual_cnt > count_max)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Sector count too large for LBA28. Truncating:");
        kprintf("   LBA: 0x%x, Count: %u => %u", actual_lba, actual_cnt, count_max);
        actual_cnt = count_max;
    }

    return ahci_write_sectors_dma(driver, dev, 
        actual_lba, 
        actual_cnt, 
        ioctl_write->io_buffer_base);
}

static int ahci_write_sectors_dma_ext(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        true, // Write => write = 1
        true, // Use the LBA param
        ATA_CMD_WRITE_DMA_EXT, 
        lba, 
        count, 
        buffer);
}

static int ahci_write_sectors_dma(struct generic_driver_t* driver, ahci_dev_t* dev, u64 lba, u16 count, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        true, // Write => write = 1
        true, // Use the LBA param
        ATA_CMD_WRITE_DMA, 
        lba, 
        count, 
        buffer);
}

//////////////////////////
//// IDENTIFY
//////////////////////////

static int ahci_identify_ata(struct generic_driver_t* driver, ahci_dev_t* dev, void* buffer);
static int ahci_identify_atapi(struct generic_driver_t* driver, ahci_dev_t* dev, void* buffer);

static int ahci_identify(struct generic_driver_t* driver, ahci_dev_t* dev, void* ctx)
{
    u8 dev_type = ahci_device_get_type(dev);
    switch (dev_type)
    {
        case AHCI_PORT_TYPE_SATA:
            return ahci_identify_ata(driver, dev, ctx);
        case AHCI_PORT_TYPE_SATAPI:
            return ahci_identify_atapi(driver, dev,ctx);
        default:
            break;
    }

    driver_log_state(driver, DRIVER_LOG_WARN, "Unknown device. Cannot identify");
    return -1;
}

static int ahci_identify_ata(struct generic_driver_t* driver, ahci_dev_t* dev, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        false, // Read => write = 0
        false, // Do not use the LBA param
        ATA_CMD_IDENTIFY, 
        0, 
        1, 
        buffer);
}

static int ahci_identify_atapi(struct generic_driver_t* driver, ahci_dev_t* dev, void* buffer)
{
    return ahci_call_wrapper(
        driver, 
        dev, 
        false, // Read => write = 0
        false, // Do not use the LBA param
        ATA_CMD_IDENTIFY_PACKET, 
        0, 
        1, 
        buffer);
}
