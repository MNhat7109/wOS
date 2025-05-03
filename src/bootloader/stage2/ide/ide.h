#pragma once
#include <stdbool.h>
#include "../stdint.h"

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

struct IDE_channel_reg_t 
{
    u16 base;  // I/O Base.
    u16 ctrl;  // Control Base
    u16 bmide; // Bus Master IDE
    u8  nIEN;  // nIEN (No Interrupt);
};

struct ide_device_t 
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
};

extern struct IDE_channel_reg_t channels[2];
extern struct ide_device_t ide_devices[4];

bool IDE_init();
void IDE_write_reg(u8 channel, u8 reg, u8 value);
u8 IDE_read_reg(u8 channel, u8 reg);
u8 IDE_poll(u8 channel, u32 advanced);
u8 IDE_panic(u32 drive, u8 err);