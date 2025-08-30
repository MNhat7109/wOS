#pragma once
#include <stdbool.h>
#include <devices/ahci/ahci_defs.h>

#define AHCI_CMD_HUNG -2
#define AHCI_CMD_BUSY -1
#define AHCI_CMD_FAILURE 0
#define AHCI_CMD_SUCCESS 1
#define AHCI_CMD_UNSUPPORTED 2

#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

struct generic_driver_t;

struct ahci_dev_t;
typedef struct ahci_dev_t ahci_dev_t;

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;

int ahci_find_cmd_slot(ahci_dev_t* dev);
int ahci_ioctl(struct generic_driver_t* driver, ahci_dev_t* dev, ahci_ioctl_ops_t cmd, void* ctx);

bool ahci_handle_ioctl(struct generic_driver_t* driver, ahci_dev_t* dev, int op, void* ctx);
bool ahci_device_identify(struct generic_driver_t* driver, ahci_dev_t* device);