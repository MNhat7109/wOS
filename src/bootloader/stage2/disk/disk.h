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
} __attribute__((packed)) mbr_partition_entry_t;

typedef struct
{
    mbr_partition_entry_t* partition_offset;
    u8 ata_drive_number;
} disk_t;

extern disk_t* disk_packet;

bool disk_init(u8 ata_drive_number, void* part_offset);