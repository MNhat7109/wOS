#pragma once
#include "../../stdint.h"
#include <stdbool.h>

#define AHCI_DEV_CMD_OK     (1 << 0)  // Last command succeeded
#define AHCI_DEV_CONNECTED  (1 << 1)  // Device present
#define AHCI_DEV_ADDRMODE   (1 << 2)  // LBA or CHS
#define AHCI_DEV_LBA48      (1 << 3)  // LBA48 supported

#define AHCI_PORT_TYPE_NONE   0
#define AHCI_PORT_TYPE_SATA   1
#define AHCI_PORT_TYPE_SATAPI 2
#define AHCI_PORT_TYPE_SEMB   3
#define AHCI_PORT_TYPE_PM     4

struct ahci_dev_t;
typedef struct ahci_dev_t ahci_dev_t;

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;

struct generic_driver_t;

u8 ahci_device_get_id(ahci_dev_t* device);
u8 ahci_device_get_type(ahci_dev_t* device);

void ahci_device_set_id(ahci_dev_t* device, u8 number);
void ahci_device_set_type(ahci_dev_t* device, u8 type);

void ahci_device_set_flag(ahci_dev_t* device, u32 flag);
void ahci_device_clear_flag(ahci_dev_t* device, u32 flag);

bool ahci_device_startup(ahci_dev_t* device);
bool ahci_device_shutdown(ahci_dev_t* device);
bool ahci_device_reset(ahci_dev_t* device);

bool ahci_device_wait_complete(ahci_dev_t* device, int slot);

void ahci_device_detect(struct generic_driver_t* driver, ahci_controller_t* self);
void ahci_device_reset_all(struct generic_driver_t* driver, ahci_controller_t* self);

bool ahci_device_rebase(struct generic_driver_t* driver, ahci_dev_t* device);
bool ahci_device_nuke(struct generic_driver_t* driver, ahci_dev_t* device);
