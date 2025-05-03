#include "disk.h"
#include "../ide/ide.h"
#include "../ide/ata.h"

disk_t* disk_packet;

bool disk_init(u8 ata_drive_number, void* part_offset)
{
    if (!IDE_init()) return false;
    
    disk_t pack =
    {
        .partition_offset=(mbr_partition_entry_t*)part_offset,
        .ata_drive_number=ata_drive_number
    };
    disk_packet=(disk_t*)&pack;
}