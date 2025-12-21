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

int partition_read(disk_t* disk, int partition, u32 lba, u32 count, void* buffer)
{
    if (disk->partition_table_type==PART_TABLE_TYPE_UNKNOWN) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Cannot identify partition table for disk %s%u\n",
        str_media[disk->media_type], disk->pos);
        return -EINVAL;
    }

    disk_set(disk->pos);
    partition_t* partition_list = (partition_t*)disk->partition_data;
    u32 read_lba_start = partition_list[partition].lba_start+lba;

    if (lba >= partition_list[partition].total_sector_count)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_DISK, "Attempted to read past end of partition\n");
        return -EINVAL;
    }

    return disk_read(read_lba_start, count, buffer);
}
int partition_write(disk_t* disk, int partition, u32 lba, u32 count, void* buffer);

int partition_table_add_entry(disk_t* disk, u32 start_lba, u32 total_sectors, u8 bootable, u8 type, u32 magic)
{
    if (disk->partition_count >= MAX_PART_COUNT) return 1;

    partition_t* partition_list = (partition_t*)disk->partition_data;
    partition_list[disk->partition_count++] = (partition_t){
        .lba_start = start_lba,
        .total_sector_count = total_sectors,
        .attributes = {
            .active = bootable,
            .type = type
        },
        .magic = magic
    };
}
