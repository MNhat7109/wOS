#pragma once
#include "../stdint.h"

#define MODULE_DISK "DISK"

typedef struct disk_ops_t disk_ops_t;
typedef struct disk_t disk_t;

typedef enum
{
    PART_TABLE_TYPE_UNKNOWN,
    PART_TABLE_TYPE_MBR,
    PART_TABLE_TYPE_GPT,
} disk_part_table_type_t;

typedef enum 
{ 
    MEDIA_TYPE_CDROM,
    MEDIA_TYPE_HD,
    MEDIA_TYPE_FD,
    MEDIA_TYPE_USB,
} disk_media_type_t;

typedef enum
{
    CTL_TYPE_IDE,
} disk_ctl_type_t;

typedef struct disk_ops_t
{
    int (*read)(disk_t* disk, u32 lba, u32 count, void* buffer);
    int (*write)(disk_t* disk, u32 lba, u32 count, void* buffer);
} disk_ops_t;

typedef struct disk_t
{
    u8 occupied;
    u8 pos;

    u32 total_sectors;
    int ctl_type;
    void* ctrl;
    int media_type;
    u32 drive_number;
    disk_ops_t* ops;
    
    int partition_table_type;
    u8 partition_count;
    u8 partition_data[2048];
} disk_t;

int disk_init();

int disk_find_boot_dev(void* lba0_buffer);
int disk_get(u32 disk_number, disk_t** out);
void disk_rescan_all(int controller_type);