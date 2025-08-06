#include "disk_ahci.h"
#include "disk_defs.h"

#include "../devices/ahci/ahci.h"
#include "../devices/ahci/ahci_defs.h"
#include "../devices/ahci/ahci_device.h"
#include "../string/string.h"
#include "../stdio.h"

static struct
{
    struct ahci_driver_t* ahci_dev;
    int fail_cnt;
    u8 buffer[512];
} disk_ahci;

extern struct disk_shared_t disk;

void disk_ahci_receive(ahci_controller_t* ctl, u32 ctl_count)
{
    for (u32 i=0;i<ctl_count;i++)
    {
        ahci_controller_t* ctler = &ctl[i];
        for (u32 j=0;j<ctler->device_count;j++)
        {
            ahci_dev_t* dev = &ctler->devices[j];
            u8 dev_type = ahci_device_get_type(dev);
            
            // Bound check mounted drives
            if (disk.entry_count >= MAX_DISK_MOUNT_ENTRY)
            {
                kprintf("Disk: Max mounted disk entry exceeded\n");
                continue;
            }

            // Register the device
            disk_t* drive = &disk.disk_mount[disk.entry_count];
            u8 high_bits = 0;
            if (dev_type == AHCI_PORT_TYPE_SATA) 
            high_bits = DISK_MEDIA_TYPE_HDD;
            else high_bits = DISK_MEDIA_TYPE_OPTICAL;
            drive->dev_type = (DISK_TYPE_AHCI&0xF) | ((high_bits&0xF)<<4);
            drive->dev_drive_number = disk.entry_count;
            drive->sector_size = (high_bits==DISK_MEDIA_TYPE_OPTICAL)?2048:512;
            drive->disk_data = (void*)dev;
            disk.entry_count++;

            kprintf("Disk: Registered drive with media type: %s, controller type: AHCI, as drive number %u\n"
            , (const char*[]){"HDD", "Optical"}[high_bits], drive->dev_drive_number);

            // Read MBR partition info, only if it's a hard drive (HDD, SSD, etc.)
            if (dev_type != AHCI_PORT_TYPE_SATA) continue;

            ahci_ioctl_rw_t read_ctx = {
                .lba = 0,
                .sector_count=1,
                .io_buffer_base = disk_ahci.buffer
            };

            bool status = disk_ahci.ahci_dev->ioctl(&disk_ahci.ahci_dev->driver_hdr, dev, AHCI_IOCTL_READ, &read_ctx);
            if (!status)
            {
                kprintf("Disk: Cannot read partition info! Skipping...\n");
                disk_ahci.fail_cnt++;
                continue;
            }

            memcpy(drive->partition_offsets, disk_ahci.buffer+0x1BE, sizeof(drive->partition_offsets));
            memset(disk_ahci.buffer, 0, 512);
        }
    }
}

bool disk_set_up_ahci()
{
    memset(disk_ahci.buffer, 0, 512);
    disk_ahci.ahci_dev = (struct ahci_driver_t*)driver_get("AHCI");

    if (!disk_ahci.ahci_dev)
    {
        kprintf("Disk: AHCI driver not found\n");
        return false;
    }
    if (!driver_run(&disk_ahci.ahci_dev->driver_hdr))
    {
        kprintf("Disk: AHCI driver failed to start\n");
        return false;
    }

    disk_ahci.fail_cnt=0;  
    disk_ahci.ahci_dev->export_dev(&disk_ahci.ahci_dev->driver_hdr, disk_ahci_receive);
    
    if (disk_ahci.fail_cnt!=0)
    {
        kprintf("Disk: Export completed with errors.\n");
        return false;
    }

    return true;
}

bool disk_ahci_read_sectors(disk_t* drive, u64 lba, u16 count, void* buffer)
{
    u8 media_type = drive->dev_type>>4;
    ahci_dev_t* device = (ahci_dev_t*)drive->disk_data;
    bool status=true;
    u8* target_buffer = (u8*)buffer;

    // TODO: Add optical driver support
    int cmd = (media_type == DISK_MEDIA_TYPE_HDD)?AHCI_IOCTL_READ:AHCI_IOCTL_PLACEHOLDER;

    while (count>0)
    {
        ahci_ioctl_rw_t ctx = {
            .lba = lba,
            .sector_count = count<0xFF?count:0xFF,
            .io_buffer_base = target_buffer
        };
    
        status = disk_ahci.ahci_dev->ioctl(&disk_ahci.ahci_dev->driver_hdr, device, cmd, &ctx);
        target_buffer+=drive->sector_size*ctx.sector_count;

        if (!status)
        {
            kprintf("Disk: Read failed\n");
            return false;
        }

        count-=0xFF;
    }

    return status;
}

bool disk_ahci_write_sectors(disk_t* drive, u64 lba, u16 count, void* buffer)
{    
    u8 media_type = drive->dev_type>>4;
    ahci_dev_t* device = (ahci_dev_t*)drive->disk_data;
    bool status=true;
    u8* target_buffer = (u8*)buffer;

    // TODO: Add optical driver support
    int cmd = (media_type == DISK_MEDIA_TYPE_HDD)?AHCI_IOCTL_WRITE:AHCI_IOCTL_PLACEHOLDER;

    while (count>0)
    {
        ahci_ioctl_rw_t ctx = {
            .lba = lba,
            .sector_count = count<0xFF?count:0xFF,
            .io_buffer_base = target_buffer
        };
    
        status = disk_ahci.ahci_dev->ioctl(&disk_ahci.ahci_dev->driver_hdr, device, cmd, &ctx);
        target_buffer+=drive->sector_size*ctx.sector_count;

        if (!status)
        {
            kprintf("Disk: Read failed\n");
            return false;
        }

        count-=0xFF;
    }

    return status;
}