#include "disk.h"
#include "../stdio.h"
#include "../errno.h"

#include "../drivers/ide.h"
#include "../drivers/ata.h"
#include "../drivers/ata_defs.h"

extern const char* const str_media[];
extern const char* const str_ctler[];

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, disk_ops_t* ops);
void disk_unmount(disk_t* disks, u32 drive_number);

int disk_ide_read(disk_t* disk, u32 lba, u32 count, void* buffer);
int disk_ide_write(disk_t* disk, u32 lba, u32 count, void* buffer);
int disk_populate_ide(disk_t* disks);

static const disk_ops_t ide_ops = {
    .read = &disk_ide_read,
    .write = &disk_ide_write
};

int disk_ide_read(disk_t* disk, u32 lba, u32 count, void* buffer)
{
    int ret=0, read_status;
    u32 read_lba=lba, left_count=count;
    while (left_count)
    {
        u32 block=(left_count<255)?left_count:255;
        
        switch (disk->media_type)
        {
            case MEDIA_TYPE_HD:
                read_status = ata_ioctl(
                    (ide_controller_t*)disk->ctrl, 
                    ATA_ACCESS_READ, 
                    disk->drive_number, 
                    read_lba,
                    block, 
                    buffer
                );
                if (read_status < 0)
                {                    
                    ret = -ECHECKFAIL; 
                    if (read_status == -EDEVFAULT) 
                        disk->occupied=0;
                    goto done;
                }
                break;
            default:
                ret=-ENODEV;
                goto done;
        }

        read_lba+=block; left_count-=block; buffer=(void*)((u8*)buffer+block*512);
    }
done:
    return ret;
}

int disk_ide_write(disk_t* disk, u32 lba, u32 count, void* buffer)
{
    int ret=0, write_status;
    u32 write_lba=lba, left_count=count;
    while (left_count)
    {
        u32 block=(left_count<255)?left_count:255;
        
        switch (disk->media_type)
        {
            case MEDIA_TYPE_HD:
                write_status = ata_ioctl(
                    (ide_controller_t*)disk->ctrl, 
                    ATA_ACCESS_WRITE, 
                    disk->drive_number, 
                    write_lba,
                    block, 
                    buffer
                );
                if (write_status < 0)
                {                    
                    ret = -ECHECKFAIL; 
                    if (write_status == -EDEVFAULT) 
                        disk->occupied=0;
                    goto done;
                }
                break;
            default:
                ret=-ENODEV;
                goto done;
        }

        write_lba+=block; left_count-=block; buffer=(void*)((u8*)buffer+block*512);
    }
done:
    return ret;
}

int disk_rescan_ide(disk_t* disks)
{
    ide_scan_controllers();
    disk_populate_ide(disks);
}

int disk_populate_ide(disk_t* disks)
{
    ide_controller_t* ide_ctl; int pos=0;
    while (ide_export_controller(&pos, &ide_ctl)==0)
    {
        for (int i=0;i<4;i++)
        {
            ide_device_t* dev = &ide_ctl->ide_devices[i];
            if (!dev->_reserved) continue;
            
            u32 drive_pos = disk_mount(
                disks, 
                CTL_TYPE_IDE, 
                dev->type==IDE_ATA?MEDIA_TYPE_HD:MEDIA_TYPE_CDROM,
                &ide_ops
            );

            disk_t* new_disk = &disks[drive_pos];
            new_disk->ctrl = (void*)ide_ctl;
            new_disk->drive_number = dev->drive;
            new_disk->total_sectors = dev->size;

            kdebugf(DEBUG_INFO, MODULE_DISK, "\tDrive %s%u, at controller %s%d, is now online.\n",
            str_media[new_disk->media_type], drive_pos, str_ctler[new_disk->ctl_type], pos);
        }
        pos++;
    }
}

int disk_try_ide(disk_t* disks)
{
    kdebugf(DEBUG_INFO, MODULE_DISK, "Checking for IDE controller(s)...\n");
    int status = ide_init();
    if (status < 0) 
    {
        kdebugf(DEBUG_INFO, MODULE_DISK, "No IDE controllers can be found.\n");
        return -1;
    }

    ide_controller_t* ide_ctl; int pos=0;
    kdebugf(DEBUG_INFO, MODULE_DISK, "IDE check done. Connecting disks...\n");
    
    disk_populate_ide(disks);
    return 0;
}
