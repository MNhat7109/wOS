#include "disk.h"
#include "../devices/driver.h"
#include "../devices/ahci/ahci.h"
#include "../stdio.h"

#define MAX_DISK_MOUNT_ENTRY 256
#define DISK_TYPE_AHCI 1
#define DISK_TYPE_IDE 2

disk_t disk_mount[MAX_DISK_MOUNT_ENTRY];
int current_entry_count=0;
int current_disk = 0;
struct ahci_driver_t* ahci_disk_driver = NULL;

bool disk_prepare_ahci()
{
    ahci_disk_driver = (struct ahci_driver_t*)driver_get("AHCI");
    if (!ahci_disk_driver)
    {
        return false;
    }
    if (!ahci_disk_driver->driver_hdr.probe((struct generic_driver_t*)ahci_disk_driver))
    {
        return false;
    }

    ahci_disk_driver->driver_hdr.config((struct generic_driver_t*)ahci_disk_driver);
    
    for (int i=0;i<ahci_disk_driver->ports.current_device_count;i++)
    {
        disk_mount[current_entry_count].dev_type = DISK_TYPE_AHCI;
        disk_mount[current_entry_count].dev_drive_number = i;
        // disk_mount[current_entry_count].partition_offsets;
    }
    return true;
}

bool disk_init()
{
    /* FIXME: What the hell is this */
    bool ok = true;
    driver_load(ahci_get_driver);
    //driver_load(ide_get_driver);
    bool ahci_ready = disk_prepare_ahci();
    //bool ide_ready = disk_prepare_ide();
    ok = ok && ahci_ready;
    //ok = ok && ide_ready;
    if (!ahci_ready)
    {
        // Print...
        kprintf("Disk: AHCI controller not found\n");
    }
    //TODO

    if (!ok)
    {
        kprintf("Disk: No drivers are loaded.\n");
        return false;
    }
    return true;
}

bool disk_change_drive(int drive_number)
{
    current_disk = drive_number;
}

bool disk_read_sectors(u32 lba, u32 count, void* buffer)
{
    
}

bool disk_write_sectors(u32 lba, u32 count, void* buffer)
{

}