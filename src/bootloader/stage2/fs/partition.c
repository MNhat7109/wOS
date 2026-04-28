#include "partition.h"
#include <stdbool.h>
#include "../stdio.h"
#include "../errno.h"
#include "mbr.h"
#include "disk.h"

extern const char* const str_media[];
extern const char* const str_ctler[];

int partition_table_setup(disk_t* disk)
{
    int status = mbr_setup(disk);
    bool gpt = false;

    switch (status)
    {
        case 0:
            disk->partition_table_type = PART_TABLE_TYPE_MBR;
            break;
        case 1:
            gpt=true;
            break;
        default:
            break;
    }

    if (!gpt) goto end;
    
    // TODO
    kdebugf(DEBUG_WARN, MODULE_DISK, "GPT not implemented yet. Sorry!\n");
end:
    return status;
}

int partition_get(disk_t* disk, int partition_number, partition_t** out)
{
    partition_t* partition_list = (partition_t*)disk->partition_data;
    partition_t* selected = &partition_list[partition_number];

    if (disk->partition_table_type == PART_TABLE_TYPE_UNKNOWN)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Partition table in disk %s%u does not exist\n",
        str_media[disk->media_type], disk->pos);
        return -EINVAL;
    }

    if (!selected->magic)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Partition %s%up%u is corrupted\n",
        str_media[disk->media_type], disk->pos, selected->attributes.pos);
        return -1;
    }

    *out = &partition_list[partition_number];
    return 0;
}

int partition_read(disk_t* disk, partition_t* part, u32 lba, u32 count, void* buffer)
{
    if (disk->partition_table_type==PART_TABLE_TYPE_UNKNOWN) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot identify partition table for disk %s%u\n",
            str_media[disk->media_type], disk->pos);
            return -EINVAL;
    }
    
    u32 read_lba_start = part->lba_start+lba;
    
    kdebugf_silent(DEBUG_INFO, MODULE_DISK, "Reading %u sector(s) from %s%up%u... to 0x%x\n",
    count, str_media[disk->media_type], disk->pos, part->attributes.pos, buffer);

    if (lba >= part->total_sector_count)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Attempted to read past end of partition\n");
        return -EINVAL;
    }

    while (count)
    {
        u16 blk = (count < 128)? count:128;
        int status;
        if ((status=disk->ops->read(disk, read_lba_start, count, buffer))!=0)
        {
            kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Reading disk failed.\n");
            return -ECMDFAIL;
        }
        lba+=blk; count-=blk; buffer=(u8*)buffer+blk*0x200;
    }
    return 0;
}

int partition_write(disk_t* disk, partition_t* part, u32 lba, u32 count, void* buffer)
{
    if (disk->partition_table_type==PART_TABLE_TYPE_UNKNOWN) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot identify partition table for disk %s%u\n",
        str_media[disk->media_type], disk->pos);
        return -EINVAL;
    }

    u32 write_lba_start = part->lba_start+lba;

    kdebugf_silent(DEBUG_INFO, MODULE_DISK, "Writing %u sector(s) to %s%up%u...\n",
    count, str_media[disk->media_type], disk->pos, part->attributes.pos);
    
    if (lba >= part->total_sector_count)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Attempted to write past end of partition\n");
        return -EINVAL;
    }

    while (count)
    {
        u16 blk = (count < 128)? count:128;
        int status;
        if ((status=disk->ops->write(disk, write_lba_start, count, buffer))!=0)
        {
            kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Writing to disk failed.\n");
            return -ECMDFAIL;
        }
        lba+=blk; count-=blk; buffer=(u8*)buffer+blk*0x200;
    }
    return 0;
}

int partition_table_add_entry(disk_t* disk, u32 start_lba, u32 total_sectors, u8 bootable, u8 type, u32 magic)
{
    if (disk->partition_count >= MAX_PART_COUNT) return 1;

    partition_t* partition_list = (partition_t*)disk->partition_data;
    partition_list[disk->partition_count] = (partition_t){
        .lba_start = start_lba,
        .total_sector_count = total_sectors,
        .attributes = {
            .active = bootable,
            .type = type,
            .pos = disk->partition_count
        },
        .magic = magic
    };
    disk->partition_count++;
}
