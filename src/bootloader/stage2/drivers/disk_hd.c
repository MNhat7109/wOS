#include "disk.h"
#include "../stdio.h"

#include "ide.h"
#include "ata.h"
#include "ata_defs.h"

extern const char* const str_media[];
extern const char* const str_ctler[];

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, void* ctl, u32 ctl_drive_number);
void disk_unmount(disk_t* disks, u32 drive_number);

int disk_populate_ide(disk_t* disks);

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
                ide_ctl, 
                dev->drive
            );
            disk_t* new_disk = &disks[drive_pos];
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
