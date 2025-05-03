#include "ata.h"
#include "ide.h"
#include "../stdio.h"
#include "../io/io.h"

u8 ATA_access_drive(bool direction, u8 drive_number, u32 lba, u8 count, u8 selector, u32 address)
{
    if (!(ide_devices[drive_number].capabilities&0x200)) 
    {
        kprintf("ATA_access: LBA support is required\n");
        return 100;
    }

    u8 lba_mode, lba_io[6], head, err;
    u32 channel = ide_devices[drive_number].channel, 
    slave_bit = ide_devices[drive_number].drive, bus = channels[channel].base,
    words=0x100;

    IDE_write_reg(channel, ATA_REG_CONTROL, 2);

    if (lba >= 0x10000000)
    {    
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
    while (IDE_read_reg(channel, ATA_REG_STATUS)&ATA_SR_BSY);

    IDE_write_reg(channel, ATA_REG_HDDEVSEL, 0xE0 | (slave_bit<<4) | head);

    if (lba_mode==2)
    {
        IDE_write_reg(channel, ATA_REG_SECCOUNT1, 0);
        IDE_write_reg(channel, ATA_REG_LBA3, lba_io[3]);
        IDE_write_reg(channel, ATA_REG_LBA4, lba_io[4]);
        IDE_write_reg(channel, ATA_REG_LBA5, lba_io[5]);
    }
    IDE_write_reg(channel, ATA_REG_SECCOUNT0, count);
    IDE_write_reg(channel, ATA_REG_LBA0, lba_io[0]);
    IDE_write_reg(channel, ATA_REG_LBA1, lba_io[1]);
    IDE_write_reg(channel, ATA_REG_LBA2, lba_io[2]);

    u8 cmd;
    if (lba_mode==1) 
    {
        if (direction==0) cmd = ATA_CMD_READ_PIO;
        else cmd = ATA_CMD_WRITE_PIO;
    }
    else
    {
        if (direction==0) cmd = ATA_CMD_READ_PIO_EXT;
        else cmd = ATA_CMD_WRITE_PIO_EXT;
    }
    IDE_write_reg(channel, ATA_REG_COMMAND, cmd);
    
    if (direction==false)
    {
        // Read
        for (int i=0;i<count;i++)
        {
            if (err=IDE_poll(channel, 1))
            {
                return err;
            }
            insw_far(bus, selector, (void*)address, words);
            address+=(2*words);
        }
    }
    else
    {
        for (int i=0;i<count;i++)
        {
            IDE_poll(channel, 0);
            outsw_far(bus, selector, (void*)address, words);
            address+=(2*words);
        }
        IDE_write_reg(channel, ATA_REG_COMMAND, 
            (u8[]){ATA_CMD_CACHE_FLUSH,ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
        IDE_poll(channel, 0);
    }
    return 0;
}

u8 ATA_read_drive(u8 drive_number, u32 lba, u16 count, void* buffer)
{
    u8 status;
    //u8 to_read = (count < 256)?count:0;
    if (drive_number>3||ide_devices[drive_number]._reserved==0) return 1;
    if (lba+count>ide_devices[drive_number].size) return 2;

    status = ATA_access_drive(0, drive_number, lba, count, 0x10, (u32)buffer);
    return IDE_panic(drive_number, status);
}