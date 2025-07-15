#pragma once
#include "../../stdint.h"
#include <stdbool.h>

#define AHCI_PORT_NONE   0
#define AHCI_PORT_SATA   1
#define AHCI_PORT_SATAPI 2
#define AHCI_PORT_SEMB   3
#define AHCI_PORT_PM     4

#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_ERR     0x01    // Error

extern bool boh_support, pm_support, emb_support;