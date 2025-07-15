#pragma once
#include <stdbool.h>
#include "../../stdint.h"

#define AHCI_CMD_HUNG -2
#define AHCI_CMD_BUSY -1
#define AHCI_CMD_FAILURE 0
#define AHCI_CMD_SUCCESS 1
#define AHCI_CMD_UNSUPPORTED 2

struct ahci_driver_t;
typedef struct ahci_driver_t ahci_driver_t;

struct ahci_device_entry_t;
typedef struct ahci_device_entry_t ahci_device_entry_t;

int ahci_find_avl_cmd_slots(ahci_device_entry_t* port);

int ahci_identify(ahci_driver_t* self, ahci_device_entry_t* port, void* out_buffer);
int ahci_read_sectors(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer);
int ahci_write_sectors(ahci_driver_t* self, ahci_device_entry_t* port, u64 lba, u16 count, void* buffer);