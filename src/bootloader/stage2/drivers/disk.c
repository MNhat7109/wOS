#include "disk.h"
#include "ide.h"
#include "../errno.h"
#include "../stdio.h"
#include "../containers/queue.h"

static struct
{
    disk_t disks[32];
    u32 disk_count;
    QUEUE(u32) free_drive_num_list;
    u32 free_list_data[32];
} disk_data;

#define MODULE_DISK "DISK"

static const char* str_media[] = {
    "cd",
    "hd",
    "fd",
    "usb",
};

static const char* str_ctler[] = {
    "ide",
};

// TODO: disk_try_every_ctler_i_can_think_of()

int disk_try_ide();

u32 disk_allocate_number();
void disk_free_number(u32 drive_num);

void disk_init()
{
    QUEUE_INIT(disk_data.free_drive_num_list, 32, disk_data.free_list_data);
    // TODO
    // if (disk_try_ahci()>0)
    // {
    //     // Do something AHCI-related
    //     goto next;
    // }

    if (disk_try_ide()>0)
        goto next;

    // And so on...
next:
}

int disk_try_ide()
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
    
    while (ide_export_controller(&pos, &ide_ctl)==0)
    {
        for (int i=0;i<4;i++)
        {
            ide_device_t* dev = &ide_ctl->ide_devices[i];
            if (!dev->_reserved) continue;
            u32 drive_pos = disk_allocate_number();
            disk_t* new_disk = &disk_data.disks[drive_pos];
            *new_disk = (disk_t){
                .ctl_type = CTL_TYPE_IDE,
                .ctrl = ide_ctl,
                .drive_number = dev->drive,
                .media_type = dev->type==IDE_ATA?MEDIA_TYPE_HD:MEDIA_TYPE_CDROM
            };
            kdebugf(DEBUG_INFO, MODULE_DISK, "\tDrive %s%u, at controller %s%d, is now online.\n",
            str_media[new_disk->media_type], drive_pos, str_ctler[new_disk->ctl_type], pos);
        }
    }
    return 0;
}

u32 disk_allocate_number()
{
    u32 number;
    if (!QUEUE_EMPTY(disk_data.free_drive_num_list))
    {
        number = QUEUE_FRONT(disk_data.free_drive_num_list, u32);
        QUEUE_POP(disk_data.free_drive_num_list);
        goto done;
    }

    number = disk_data.disk_count++;
done:
    return number;
}

void disk_free_number(u32 drive_num)
{
    QUEUE_PUSH(disk_data.free_drive_num_list, u32, drive_num);
}