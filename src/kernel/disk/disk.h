#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    u8 active;
    u8 chs_start[3];
    u8 system_id;
    u8 chs_end[3];
    u32 lba_start;
    u32 total_sectors;
} __attribute__((packed)) partition_entry_t;

typedef struct
{
    int dev_type;
    int dev_drive_number;
    partition_entry_t* partition_offsets[4];
} __attribute__((packed)) disk_t;

bool disk_init();
bool disk_change_drive(int drive_number);
bool disk_read_sectors(u32 lba, u32 count, void* buffer);
bool disk_write_sectors(u32 lba, u32 count, void* buffer);