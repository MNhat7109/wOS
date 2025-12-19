#include "disk.h"
#include "ide.h"
#include "ata.h"
#include "ata_defs.h"
#include "../errno.h"
#include "../stdio.h"
#include "../string/string.h"
#include "../containers/queue.h"

static struct
{
    disk_t disks[32];
    u32 disk_count;
    u32 current_disk;
    QUEUE(u32) free_drive_num_list;
    u32 free_list_data[32];
} disk_data;

const char* const str_media[] = {
    "cd",
    "hd",
    "fd",
    "usb",
};

const char* const str_ctler[] = {
    "ide",
};

// TODO: disk_try_every_ctler_i_can_think_of()

int disk_try_ide(disk_t* disks);
int disk_rescan_ide(disk_t* disks);

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, void* ctl, u32 ctl_drive_number);
void disk_unmount(disk_t* disks, u32 drive_number);
u32 disk_allocate_number();
void disk_free_number(u32 drive_num);

void disk_init()
{
    QUEUE_INIT(disk_data.free_drive_num_list, 32, disk_data.free_list_data);

    if (disk_try_ide(disk_data.disks)>0)
        goto next;

    // And so on...
next:
    disk_set(0);
}

void disk_set(u32 disk_number)
{
    disk_data.current_disk = disk_number;
}

int disk_read(u32 lba, u32 count, void* buffer)
{
    disk_t* disk = &disk_data.disks[disk_data.current_disk];
    int read_status, ret=0;

    while (count)
    {
        u32 block=(count<255)?count:255;
        switch (disk->ctl_type)
        {
            case CTL_TYPE_IDE:
                read_status = ata_ioctl(
                    (ide_controller_t*)disk->ctrl, 
                    ATA_ACCESS_READ, 
                    disk->drive_number, 
                    lba,
                    block, 
                    buffer
                );
                if (read_status < 0)
                {
                    kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Reading sectors failed.\n");
                    if (read_status == -EDEVFAULT) disk->occupied=0;
                    ret = -ECHECKFAIL;
                }
                break;
            default:
                break;
        }
        if (ret < 0) break;

        lba+=block; count-=block; buffer=(void*)((u8*)buffer+block*512);
    }

    if (!disk->occupied)
    {
        disk_rescan_all(disk->ctl_type);
    }

    kdebugf(DEBUG_INFO, MODULE_DISK, "Reading sectors done.\n");

    return ret;
}

void disk_rescan_all(int controller_type)
{
    for (u32 i=0;i<disk_data.disk_count;i++)
    {
        disk_t* disk = &disk_data.disks[i];
        if (disk->ctl_type != controller_type) continue;

        disk_unmount(disk_data.disks, i);
    }

    switch (controller_type)
    {
        case CTL_TYPE_IDE:
            disk_rescan_ide(disk_data.disks);
            break;
        default:
            break;
    }

}

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, void* ctl, u32 ctl_drive_number)
{
    u32 drive_pos = disk_allocate_number();
    disk_t* new_disk = &disks[drive_pos];
    *new_disk = (disk_t){
        .ctl_type = ctl_type,
        .ctrl = ctl,
        .drive_number = ctl_drive_number,
        .media_type = media_type,
        .occupied = 1
    };
    return drive_pos;
}

void disk_unmount(disk_t* disks, u32 drive_number)
{
    disk_t* disk = &disks[drive_number];
    memset(disk, 0, sizeof(disk_t));

    disk_free_number(drive_number);
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
    disk_t* disk = &disk_data.disks[drive_num];
    if (!disk->occupied) return;
    QUEUE_PUSH(disk_data.free_drive_num_list, u32, drive_num);
}