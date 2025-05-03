#pragma once
#include "../stdint.h"
#include <stdbool.h>

u8 ATA_read_drive(u8 drive_number, u32 lba, u16 count, void* buffer);
