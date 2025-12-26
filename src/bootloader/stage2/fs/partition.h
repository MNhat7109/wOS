#pragma once
#include "../stdint.h"

#define MAX_PART_COUNT 128

typedef enum
{
    PARTITION_TYPE_PRIMARY,
    PARTITION_TYPE_EXTENDED,
    PARTITION_TYPE_LOGICAL,
} partition_type_t;

typedef struct partition_t
{
    u32 lba_start;
    u32 total_sector_count;
    struct {
        u8 active : 1;
        u8 type : 7;
        u8 pos : 8;
    } __attribute__((packed)) attributes;
    u32 magic;
} partition_t;

typedef struct disk_t disk_t;
int partition_table_setup(disk_t* disk);

int partition_read(disk_t* disk, int partition, u32 lba, u32 count, void* buffer);
int partition_write(disk_t* disk, int partition, u32 lba, u32 count, void* buffer);