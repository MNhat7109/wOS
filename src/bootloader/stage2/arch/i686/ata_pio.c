#include "../../stdint.h"
#include "../../stdio.h"
#include "../../drivers/ide.h"
#include "../../drivers/ata_defs.h"
#include "../../drivers/ata.h"
#include "../../errno.h"
#include "io.h"

typedef struct ide_controller_t ide_controller_t;

void ide_write_reg(ide_controller_t* ctrl, u8 channel, u8 reg, u8 value);
u8 ide_read_reg(ide_controller_t* ctrl, u8 channel, u8 reg);
int ide_error_check(ide_controller_t* ctrl, u32 drive, u8 initial_error);
int ide_poll(ide_controller_t* ctrl, u8 channel, u8 advanced);

u8 ata_pio_access_drive(ide_controller_t* ctrl, int direction, u32 drive, u32 lba, u8 count, void* address)
{
    kdebugf(DEBUG_INFO, MODULE_IDE_ATA, "Reading %u sector(s), from LBA 0x%x, at drive number %u...\n", count, lba, drive);
    if (!(ctrl->ide_devices[drive].capabilities&ATA_IDENT_CAP_LBA))
    {
        kdebugf(DEBUG_CRITICAL, MODULE_IDE_ATA, "LBA support is required\n");
        return 100;
    }

    u8 lba_mode, lba_io[6], head, err;
    u8 channel = ctrl->ide_devices[drive].channel, 
    slave = ctrl->ide_devices[drive].drive;
    u16 bus = ctrl->channels[channel].base, words=0x100;

    ide_write_reg(ctrl, channel, ATA_REG_CONTROL, 2);

    if (lba >= 0x10000000)
    {    
        if (!(ctrl->ide_devices[drive].cmd_sets&ATA_IDENT_CMDSETS_LBA48))
        {
            kdebugf(DEBUG_CRITICAL, MODULE_IDE_ATA, "LBA48 support is required to access LBA >= 0x10000000\n");
            return 100;
        }

        lba_mode=2;
        lba_io[0] = lba&0xFF;
        lba_io[1] = (lba>>8)&0xFF;
        lba_io[2] = (lba>>16)&0xFF;
        lba_io[3] = (lba>>24)&0xFF;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = 0;
    }
    else
    {
        lba_mode=1;
        lba_io[0] = lba&0xFF;
        lba_io[1] = (lba>>8)&0xFF;
        lba_io[2] = (lba>>16)&0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba>>24)&0xF;
    }
    ide_poll(ctrl, channel, 0);

    ide_write_reg(ctrl, channel, ATA_REG_HDDEVSEL, 0xE0 | (slave<<4) | head);
    ide_poll(ctrl, channel, 0);

    if (lba_mode==2)
    {
        ide_write_reg(ctrl, channel, ATA_REG_SECCOUNT1, 0);
        ide_write_reg(ctrl, channel, ATA_REG_LBA3, lba_io[3]);
        ide_write_reg(ctrl, channel, ATA_REG_LBA4, lba_io[4]);
        ide_write_reg(ctrl, channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write_reg(ctrl, channel, ATA_REG_SECCOUNT0, count);
    ide_write_reg(ctrl, channel, ATA_REG_LBA0, lba_io[0]);
    ide_write_reg(ctrl, channel, ATA_REG_LBA1, lba_io[1]);
    ide_write_reg(ctrl, channel, ATA_REG_LBA2, lba_io[2]);

    u8 cmd;
    if (lba_mode==1) 
    {
        if (direction==ATA_ACCESS_READ) cmd = ATA_CMD_READ_PIO;
        else cmd = ATA_CMD_WRITE_PIO;
    }
    else
    {
        if (direction==ATA_ACCESS_READ) cmd = ATA_CMD_READ_PIO_EXT;
        else cmd = ATA_CMD_WRITE_PIO_EXT;
    }
    ide_write_reg(ctrl, channel, ATA_REG_COMMAND, cmd);

    if (direction==ATA_ACCESS_READ)
    {
        for (int i=0;i<count;i++)
        {
            if ((err=ide_poll(ctrl, channel, 1))<0) return err;
            insw(bus, address, words);
            address+=(2*words);
        }
    }
    else
    {
        for (int i=0;i<count;i++)
        {
            ide_poll(ctrl, channel, 0);
            outsw(bus, address, words);
            address+=(2*words);
        }

        ide_write_reg(ctrl, channel, ATA_REG_COMMAND,
            (u8[]){ATA_CMD_CACHE_FLUSH,ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]  
        );
        ide_poll(ctrl, channel, 0);
    }
    return 0;
}

int ata_pio_ioctl(ide_controller_t* ctrl, int direction, u32 drive, u32 lba, u16 count, void* buffer)
{
    if (drive > 3 || ctrl->ide_devices[drive]._reserved==0) return -ENODEV;
    if (lba+count>=ctrl->ide_devices[drive].size||
        lba>=ctrl->ide_devices[drive].size) return -EOOB;
    
    u8 io_status = ata_pio_access_drive(ctrl, direction, drive, lba, count, buffer);
    return ide_error_check(ctrl, drive, io_status);
}
