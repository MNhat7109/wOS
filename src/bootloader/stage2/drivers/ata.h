#pragma once
#include "../stdint.h"

#define MODULE_IDE_ATA "IDE_ATA"

typedef struct ide_controller_t ide_controller_t;
int ata_ioctl(ide_controller_t* ctrl, int direction, u32 drive, u32 lba, u16 count, void* buffer);