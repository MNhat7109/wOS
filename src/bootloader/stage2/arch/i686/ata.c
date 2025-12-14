#include "../../stdint.h"
#include "../../stdio.h"
#include "../../drivers/ide.h"

int ata_pio_ioctl(ide_controller_t* ctrl, int direction, u32 drive, u32 lba, u16 count, void* buffer);

int ata_ioctl(ide_controller_t* ctrl, int direction, u32 drive, u32 lba, u16 count, void* buffer)
{
    return ata_pio_ioctl(ctrl, direction, drive, lba, count, buffer);
}