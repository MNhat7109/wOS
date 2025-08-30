#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

bool disk_init();
bool disk_change_drive(int drive_number);
bool disk_read_sectors(u32 lba, u32 count, void* buffer);
bool disk_write_sectors(u32 lba, u32 count, void* buffer);