#pragma once
#include "../../stdint.h"
#include <stdbool.h>

#define FIS_TYPE_REG_H2D   0x27
#define FIS_TYPE_REG_D2H   0x34
#define FIS_TYPE_DMA_ACT   0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA      0x46
#define FIS_TYPE_BIST      0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS  0xA1

typedef struct
{
    u8 fis_type;

    u8 port_multi_port : 4;
    u8 _reserved0 : 3;
    u8 cmd_ctl : 1;

    u8 command;
    u8 feature_lo;

    u8 lba0;
    u8 lba1;
    u8 lba2;
    u8 device;

    u8 lba3;
    u8 lba4;
    u8 lba5;
    u8 feature_hi;

    u8 count_lo;
    u8 count_hi;
    u8 icc;
    u8 control;

    u8 _reserved1[4];
} __attribute__((packed)) fis_reg_h2d_t;

typedef struct
{
    u8 fis_type;

    u8 port_multi_port : 4;
    u8 _reserved0 : 3;
    u8 interrupt : 1;

    u8 status;
    u8 error;
    u8 lba0, lba1, lba2;
    u8 device;
    u8 lba3,lba4,lba5;
    u8 _reserved1;

    u16 count;
    u8 _reserved2[6];
} __attribute__((packed)) fis_reg_d2h_t;

void fis_reg_h2d_setup(fis_reg_h2d_t* cmd_fis, u8 command, u64 lba, u32 count, bool lba48);
