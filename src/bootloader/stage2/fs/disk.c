#include "disk.h"
#include "partition.h"
#include "../drivers/ata.h"
#include "../drivers/ata_defs.h"
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

const char* const str_pt[] = {
    "unknown",
    "mbr",
    "gpt"
};

// TODO: disk_try_every_ctler_i_can_think_of()

int disk_try_ide(disk_t* disks);
int disk_rescan_ide(disk_t* disks);

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, disk_ops_t* ops);
void disk_unmount(disk_t* disks, u32 drive_number);
u32 disk_allocate_number();
void disk_free_number(u32 drive_num);

int disk_init()
{
    QUEUE_INIT(disk_data.free_drive_num_list, 32, disk_data.free_list_data);

    if (disk_try_ide(disk_data.disks)>0)
        goto next;

    // And so on...
next:
    if (disk_data.disk_count ==0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot find any disks. Disk init failed.\n");
        return -ECHECKFAIL;
    }

    kdebugf(DEBUG_INFO, MODULE_DISK, "Scanning partition tables...\n");

    for (int i=0;i<disk_data.disk_count;i++) 
    {
        disk_t* disk = &disk_data.disks[i];
        if (!disk->occupied || disk->media_type != MEDIA_TYPE_HD) continue;
        partition_table_setup(disk);
    }
    
    for (int i=0;i<disk_data.disk_count;i++) 
    {
        disk_t* disk = &disk_data.disks[i];
        if (!disk->occupied || disk->media_type != MEDIA_TYPE_HD) continue;
        kdebugf(DEBUG_INFO, MODULE_DISK, "Disk %s%u, partition type: %s\n",
        str_media[disk->media_type], disk->pos, str_pt[disk->partition_table_type]);    
    }
    return 0;
}

u8 buffer[512];
int disk_find_boot_dev(void* lba0_buffer)
{
    if (!lba0_buffer)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Buffer is NULL\n");
        return -ECHECKFAIL;
    }

    for (u32 i=0;i<disk_data.disk_count;i++)
    {
        disk_t* disk =& disk_data.disks[i];
        if (disk->media_type != MEDIA_TYPE_HD) continue;

        int status = disk->ops->read(disk, 0, 1, buffer);
        if (status < 0) continue;
        
        if (memcmp((u8*)lba0_buffer+0x1B8, (u8*)(buffer+0x1B8), 4) == 0)
        return i;
    }
    return -1;
}

void disk_set_current_dev(u32 disk_number)
{
    disk_data.current_disk = disk_number;
}

disk_t* disk_get_current_dev()
{
    return &disk_data.disks[disk_data.current_disk];
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

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, disk_ops_t* ops)
{
    u32 drive_pos = disk_allocate_number();
    disk_t* new_disk = &disks[drive_pos];
    *new_disk = (disk_t){
        .ctl_type = ctl_type,
        .media_type = media_type,
        .occupied = 1,
        .pos = drive_pos,
        .ops = ops
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