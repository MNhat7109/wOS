#include "ahci_fis.h"

void fis_reg_h2d_setup(fis_reg_h2d_t* cmd_fis, u8 command, u64 lba, u32 count, bool lba48)
{
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->cmd_ctl = 1;
    cmd_fis->command = command;

    cmd_fis->lba0 = (u8)(lba>>0);
    cmd_fis->lba1 = (u8)(lba>>8);
    cmd_fis->lba2 = (u8)(lba>>16);
    cmd_fis->device = (1<<6);

    if (lba48)
    {
        cmd_fis->lba3 = (u8)(lba>>24);
        cmd_fis->lba4 = (u8)(lba>>32);
        cmd_fis->lba5 = (u8)(lba>>40);
    }
    else cmd_fis->device |= ((lba>>24)&0xF);

    cmd_fis->count_lo = (u8)(count);
    cmd_fis->count_hi = (u8)(count>>8);
}