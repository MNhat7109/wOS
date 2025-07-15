#pragma once
#include "../../stdint.h"
#include <stdbool.h>
#include "ahci_hba.h"

#define AHCI_MAX_PORT_ENTRIES 128

#define AHCI_DEV_CMD_OK     (1 << 0)  // Last command succeeded
#define AHCI_DEV_CONNECTED  (1 << 1)  // Device present
#define AHCI_DEV_ADDRMODE   (1 << 2)  // LBA or CHS
#define AHCI_DEV_LBA48      (1 << 3)  // LBA48 supported

#define AHCI_PORT_TYPE_NONE   0
#define AHCI_PORT_TYPE_SATA   1
#define AHCI_PORT_TYPE_SATAPI 2
#define AHCI_PORT_TYPE_SEMB   3
#define AHCI_PORT_TYPE_PM     4

// Device Entry Metadata
typedef struct ahci_device_entry_t
{
    u32 cmd_done;
    /* Bit 0: Cmd Success, 1: Connected, 2: LBA or CHS, 3: LBA48
    Bit 4-7: Port type
    Bit 8-15: Port Number*/
    u16 max_cmd_slot_count;
    u16 general_info; 
    u32 max_lba_28;
    u64 max_lba_48;
    hba_port_t* hba_port;
} __attribute__((packed)) ahci_device_entry_t;

// Ports Metadata
typedef struct ahci_ports_t
{
    u8 max_port_count;
    u8 current_device_count;
    u16 max_cmd_slot_count;
    ahci_device_entry_t devices[AHCI_MAX_PORT_ENTRIES];
} __attribute__((packed)) ahci_ports_t;

u8 ahci_port_get_number(ahci_device_entry_t* device);
u8 ahci_port_get_type(ahci_device_entry_t* device);

void ahci_port_set_number(ahci_device_entry_t* device, u8 number);
void ahci_port_set_type(ahci_device_entry_t* device, u8 type);

void ahci_port_set_flag(ahci_device_entry_t* device, u32 flag);
void ahci_port_clear_flag(ahci_device_entry_t* device, u32 flag);

void ahci_port_setup(ahci_ports_t* self, hba_memory_t* abar);
void ahci_port_detect(ahci_ports_t* self, hba_memory_t* abar);
void ahci_port_reset_all(ahci_ports_t* self, hba_memory_t* abar);

bool ahci_port_startup_dev(ahci_device_entry_t* device);
bool ahci_port_shutdown_dev(ahci_device_entry_t* device);
bool ahci_port_reset_dev(ahci_device_entry_t* device);

bool ahci_port_rebase_dev(ahci_device_entry_t* device);
bool ahci_port_nuke_dev(ahci_device_entry_t* device);