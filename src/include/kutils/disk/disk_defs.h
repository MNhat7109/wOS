#pragma once
#include <libk/stdint.h>

#define MAX_DISK_MOUNT_ENTRY 256

#define DISK_TYPE_IDE 0
#define DISK_TYPE_AHCI 1
#define DISK_TYPE_USB_MS 2
#define DISK_TYPE_NVME 3

#define DISK_MEDIA_TYPE_HDD 0 // For both SSDs and HDDs
#define DISK_MEDIA_TYPE_OPTICAL 1

#define DISK_CTL_IDE    (1<<DISK_TYPE_IDE)
#define DISK_CTL_AHCI   (1<<DISK_TYPE_AHCI)
#define DISK_CTL_USB_MS (1<<DISK_TYPE_USB_MS)
#define DISK_CTL_NVME   (1<<DISK_TYPE_NVME)

typedef struct
{
    u8 active;
    u8 chs_start[3];
    u8 system_id;
    u8 chs_end[3];
    u32 lba_start;
    u32 total_sectors;
} __attribute__((packed)) partition_entry_t;

typedef struct disk_t
{
    // Low 4 bits: Controller type, High 4 bits: Device Media Type (ATAPI, ATA)
    u8 dev_type;
    int dev_drive_number;
    u32 sector_size;
    void* disk_data;
    partition_entry_t partition_offsets[4];
} disk_t;

extern struct disk_shared_t
{
    disk_t disk_mount[MAX_DISK_MOUNT_ENTRY];
    int entry_count, current_disk_index;
    /* 8 bits here as of right now can be:
    - Bit 0: IDE controller
    - Bit 1: AHCI controller
    - Bit 2: USB Mass Storage controller
    - Bit 3: NVMe controller
    - Bit 4-7: Reserved
    */
    u8 controller_avl; 
} disk;
