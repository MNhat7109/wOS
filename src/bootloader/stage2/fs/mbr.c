#include "mbr.h"
#include "disk.h"
#include "partition.h"
#include "../errno.h"
#include "../string/string.h"
#include "../stdio.h"

#define BOOT_SIG 0xAA55

typedef struct mbr_partition_entry_t
{
    u8 active;
    u8 chs_start[3];
    u8 system_id;
    u8 chs_end[3];
    u32 lba_start;
    u32 total_sectors;
} __attribute__((packed)) mbr_partition_entry_t;

extern const char* const str_media[];
extern const char* const str_ctler[];

static struct
{
    u8 buffer[512];
} mbr_data;

int partition_table_add_entry(disk_t* disk, u32 start_lba, u32 total_sectors, u8 bootable, u8 type, u32 magic);
int mbr_parse_partition_entries(disk_t* disk, mbr_partition_entry_t* mbr_entries);
int mbr_parse_extended_entries(disk_t* disk, u32 ext_lba_base);

int mbr_setup(disk_t* disk)
{
    int ret_val=0;

    kdebugf(DEBUG_INFO, MODULE_DISK, "Setting up MBR partition table for drive %s%u...\n",
    str_media[disk->media_type], disk->pos);

    int status = disk->ops->read(disk, 0, 1, mbr_data.buffer);
    if (status < 0) 
    {
        kdebugf(DEBUG_WARN, MODULE_DISK, "Cannot set up MBR partition table for disk: %s%u.\n"
            , str_media[disk->media_type], disk->pos);
        ret_val=status;
        goto end;
    }

    if (*(u16*)(mbr_data.buffer+510) != BOOT_SIG)
    {
        kdebugf(DEBUG_WARN, MODULE_DISK, "Invalid MBR signature at disk %s%u.\n",
        str_media[disk->media_type], disk->pos);
        ret_val=-EINVAL;
        goto end;
    }

    u8* entry_start = (u8*)(mbr_data.buffer+0x1BE);
    status = mbr_parse_partition_entries(disk, (mbr_partition_entry_t*)entry_start);
    
    if (status < 0) 
    {
        kdebugf(DEBUG_WARN, MODULE_DISK, "Cannot set up MBR partition table for disk: %s%u.\n"
            , str_media[disk->media_type], disk->pos);
        ret_val=status;
        goto end;
    }

end:
    if (ret_val<0)
        kdebugf(DEBUG_WARN, MODULE_DISK, "The disk will only be accessed via raw LBA. Partition accessing is disabled.\n");
    return ret_val;
}

int mbr_parse_partition_entries(disk_t* disk, mbr_partition_entry_t* mbr_entries)
{
    partition_t* partition_list = (partition_t*)disk->partition_data;
    for (int i=0;i<4;i++)
    {
        mbr_partition_entry_t* part_entry = &mbr_entries[i];
        if (part_entry->total_sectors==0) continue;

        switch (part_entry->system_id)
        {
        case 0xEE:
            kdebugf(DEBUG_WARN, MODULE_DISK, "Protective MBR detected. Passing the job to GPT...\n");
            return 1;
        case 0xF:
        case 0x5:
        {
            int status = mbr_parse_extended_entries(disk, part_entry->lba_start);
            if (status<0) continue;
            break;
        }
        default:
        {
            int status = partition_table_add_entry(disk, part_entry->lba_start, part_entry->total_sectors,
            part_entry->active==0x80, PARTITION_TYPE_PRIMARY,MBR_MAGIC);
            if (status == 1) return 0;
            break;
        }
        }
    }
    return 0;
}

int mbr_parse_extended_entries(disk_t* disk, u32 ext_lba_base)
{
    int retval = 0;
    u32 last_ebr=0, cur_ebr = ext_lba_base;

    while (1)
    {
        int status = disk->ops->read(disk, cur_ebr, 1, mbr_data.buffer);
        if (status<0)
        {
            kdebugf(DEBUG_WARN, MODULE_DISK, "Cannot set up MBR extended partition table for disk: %s%u.\n"
            , str_media[disk->media_type], disk->pos);
            retval=status; break;
        }

        if (*(u16*)(mbr_data.buffer+510) != BOOT_SIG)
        {
            kdebugf(DEBUG_WARN, MODULE_DISK, "Invalid EBR signature at LBA=0x%x, disk %s%u.\n",
            cur_ebr, str_media[disk->media_type], disk->pos);
            retval=-EINVAL; break;
        }
        
        mbr_partition_entry_t* ebr_entries = (mbr_partition_entry_t*)(u8*)(mbr_data.buffer+0x1BE);

        if (ebr_entries[0].system_id==0 || ebr_entries[0].total_sectors == 0) goto next_ebr;

        u32 logical_lba_start = cur_ebr+ebr_entries[0].lba_start, 
        logical_sector_count = ebr_entries[0].total_sectors;

        if (logical_sector_count > disk->total_sectors-logical_lba_start)
        {
            kdebugf(DEBUG_WARN, MODULE_DISK, "Logical partition exceeds disk size\n");
            retval = -EINVAL;
            break;
        }

        status = partition_table_add_entry(
            disk, 
            cur_ebr+ebr_entries[0].lba_start,
            ebr_entries[0].total_sectors,
            0,
            PARTITION_TYPE_LOGICAL,
            MBR_MAGIC
        );
        if (status ==1) break;

next_ebr:
        if (ebr_entries[1].total_sectors == 0 || ebr_entries[1].system_id == 0) break;
        last_ebr=cur_ebr;
        cur_ebr=ext_lba_base+ebr_entries[1].lba_start;

        if (last_ebr==cur_ebr)
        {
            kdebugf(DEBUG_WARN, MODULE_DISK, "EBR of disk %s%u is corrupted.\n",
            str_media[disk->media_type], disk->pos);
            retval=-EINVAL; break;
        }
    } 

    if (retval< 0 )
        kdebugf(DEBUG_WARN, MODULE_DISK, "This extended partition will be ignored.\n");
    return retval;
}