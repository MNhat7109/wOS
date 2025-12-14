#pragma once
#include "../stdint.h"
#include "pci.h"

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

typedef struct ide_channel_reg_t 
{
    u16 base;  // I/O Base.
    u16 ctrl;  // Control Base
    u16 bmide; // Bus Master IDE
    u8  nIEN;  // nIEN (No Interrupt);
} ide_channel_reg_t;

typedef struct ide_device_t 
{
    u8  _reserved;    // 0 (Empty) or 1 (This Drive really exists).
    u8  channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    u8  drive;       // 0 (Master Drive) or 1 (Slave Drive).
    u16 type;        // 0: ATA, 1:ATAPI.
    u16 signature;   // Drive Signature
    u16 capabilities;// Features.
    u32 cmd_sets; // Command Sets Supported.
    u32 size;        // Size in Sectors.
    u8  model[41];   // Model in string.
} ide_device_t;

typedef struct ide_controller_t
{
    u32 BAR0,BAR1,BAR2,BAR3,BAR4;
    pci_device_t pci_ide;
    ide_channel_reg_t channels[2];
    ide_device_t ide_devices[4];
} ide_controller_t;

int ide_init();
int ide_export_controller(int* pos, ide_controller_t** out);