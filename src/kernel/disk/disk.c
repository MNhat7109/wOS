#include "disk.h"
#include "disk_defs.h"
#include "disk_ahci.h"

#include "../stdio.h"

struct disk_shared_t disk;

bool disk_init()
{
    disk.entry_count = 0;
    disk.current_disk_index = 0;
    disk.controller_avl = 0;

    if (disk_set_up_ahci())
    {
        disk.controller_avl |= DISK_CTL_AHCI;
        // TODO: Unload IDE driver
    }
    // TODO: Else if IDE 
    
    if (disk.controller_avl == 0)
    {
        kprintf("Disk: No disk controllers available.\n");
        return false;
    }
    return true;
}

bool disk_change_drive(int drive_number)
{
    disk.current_disk_index = drive_number;
}

bool disk_read_sectors(u32 lba, u32 count, void* buffer)
{
    if (disk.current_disk_index >= disk.entry_count) return false;

    disk_t* drive = &disk.disk_mount[disk.current_disk_index];

    bool disk_status = false;
    switch (drive->dev_type)
    {
        case DISK_TYPE_AHCI:
            disk_status = disk_ahci_read_sectors(drive, lba, count, buffer);
            break;
        default:
            break;
    }
    return disk_status;
}

bool disk_write_sectors(u32 lba, u32 count, void* buffer)
{
    if (disk.current_disk_index >= disk.entry_count) return false;

    disk_t* drive = &disk.disk_mount[disk.current_disk_index];

    bool disk_status = false;
    switch (drive->dev_type)
    {
        case DISK_TYPE_AHCI:
            disk_status = disk_ahci_write_sectors(drive, lba, count, buffer);
            break;
        default:
            break;
    }
    return disk_status;
}