#include "disk.h"
#include "../ide/ide.h"
#include "../ide/ata.h"
#include "../stdio.h"

disk_t* disk_packet;
disk_t pack;

bool disk_init(u8 ata_drive_number, void* part_offset)
{
    if (!IDE_init()) return false;
    
    pack.partition_offset=(mbr_partition_entry_t*)part_offset;
    pack.ata_drive_number=ata_drive_number;
    disk_packet=(disk_t*)&pack;
    return true;
}

bool disk_read_sectors(u32 lba, u32 count, void* buffer)
{
    u32 part_lba_start = disk_packet->partition_offset->lba_start;
    u32 size = disk_packet->partition_offset->total_sectors;
    // Do range checks
    if (lba+count > size)
    {
        kprintf("DISK: Partition size limit exceeded. Limit: %u sectors, got %u instead.\n",
        size, lba+count);
        return false;
    }

    while (count)
    {
        u16 block = count < 128 ? count : 128; u8 status;
        if ((status = ATA_read_drive(disk_packet->ata_drive_number, part_lba_start+lba, block, buffer))!=0)
        {
            // Error!
            kprintf("DISK: Reading sectors failed.\n");
            return false;
        }

        lba+=block; count-=block; buffer+=block*512;
    }
    return true;
}