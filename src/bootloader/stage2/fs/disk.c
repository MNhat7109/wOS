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

int disk_hd_read(disk_t* disk, u32 lba, u32 count, void* buffer);
int disk_hd_write(disk_t* disk, u32 lba, u32 count, void* buffer);

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, void* ctl, u32 total_sectors, u32 ctl_drive_number);
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
    kdebugf(DEBUG_INFO, MODULE_DISK, "Scanning partition tables...\n");

    for (int i=0;i<disk_data.disk_count;i++) 
    {
        disk_t* disk = &disk_data.disks[i];
        if (!disk->occupied || disk->media_type != MEDIA_TYPE_HD) continue;
        partition_table_setup(disk);
    }
    
    int first_hd_pos = -1;
    for (int i=0;i<disk_data.disk_count;i++) 
    {
        disk_t* disk = &disk_data.disks[i];
        if (!disk->occupied || disk->media_type != MEDIA_TYPE_HD) continue;
        if (first_hd_pos == -1) first_hd_pos=i;
        kdebugf(DEBUG_INFO, MODULE_DISK, "Disk %s%u, partition type: %s\n",
        str_media[disk->media_type], disk->pos, str_pt[disk->partition_table_type]);    
    }

    if (first_hd_pos<0) return;
    disk_set(first_hd_pos);
}

void disk_set(u32 disk_number)
{
    disk_data.current_disk = disk_number;
}

int disk_read(u32 lba, u32 count, void* buffer)
{
    disk_t* disk = &disk_data.disks[disk_data.current_disk];
    int read_status, ret=0;

    if (count == 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot read with a sector count of 0\n");
        ret = -EINVAL; goto done;
    }
    if (lba >= disk->total_sectors || count > disk->total_sectors - lba)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Attempted to read past the disk's total sector count\n");
        ret = -EINVAL; goto done;
    }

    switch (disk->media_type)
    {
        case MEDIA_TYPE_HD:
            ret = disk_hd_read(disk, lba, count, buffer);
            break;
        default:
            ret = -ECHECKFAIL;
            break;
    }

done:
    if (ret<0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Reading sectors failed.\n");
    }
    else
    {
        kdebugf(DEBUG_INFO, MODULE_DISK, "Reading sectors done.\n");
        kdebugf(DEBUG_INFO, MODULE_DISK, "Disk: %s%u, LBA=0x%x, Count: %u\n",
            str_media[disk->media_type], disk->pos, lba, count);
    }

    return ret;
}

int disk_write(u32 lba, u32 count, void* buffer)
{
    disk_t* disk = &disk_data.disks[disk_data.current_disk];
    int write_status, ret=0;

    if (count == 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot write with a sector count of 0\n");
        ret = -EINVAL; goto done;
    }
    if (lba >= disk->total_sectors || count > disk->total_sectors - lba)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Attempted to write past the disk's total sector count\n");
        ret = -EINVAL; goto done;
    }

    switch (disk->media_type)
    {
        case MEDIA_TYPE_HD:
            ret = disk_hd_write(disk, lba, count, buffer);
            break;
        default:
            ret = -ECHECKFAIL;
            break;
    }

done:
    if (ret<0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Writing sectors failed.\n");
    }
    else
    {
        kdebugf(DEBUG_INFO, MODULE_DISK, "Writing sectors done.\n");
        kdebugf(DEBUG_INFO, MODULE_DISK, "Disk: %s%u, LBA=0x%x, Count: %u\n",
            str_media[disk->media_type], disk->pos, lba, count);
    }

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

u32 disk_mount(disk_t* disks, int ctl_type, int media_type, void* ctl, u32 total_sectors, u32 ctl_drive_number)
{
    u32 drive_pos = disk_allocate_number();
    disk_t* new_disk = &disks[drive_pos];
    *new_disk = (disk_t){
        .ctl_type = ctl_type,
        .ctrl = ctl,
        .drive_number = ctl_drive_number,
        .media_type = media_type,
        .occupied = 1,
        .pos = drive_pos,
        .total_sectors = total_sectors
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