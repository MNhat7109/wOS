#pragma once
#include <stdbool.h>
#include <libk/stdint.h>

struct disk_t;
typedef struct disk_t disk_t;

bool disk_set_up_ahci();
bool disk_ahci_read_sectors(disk_t* drive, u64 lba, u16 count, void* buffer);
bool disk_ahci_write_sectors(disk_t* drive, u64 lba, u16 count, void* buffer);