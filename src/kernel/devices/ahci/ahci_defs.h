#pragma once
#include "../../stdint.h"

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

struct hba_port_t;
typedef struct hba_port_t hba_port_t;

typedef struct ahci_dev_t
{
    u32 cmd_done;
    u32 cmd_ok;
    u16 max_cmd_slot_count;
    /* Bit 0: Cmd Success, 1: Connected, 2: LBA or CHS, 3: LBA48
    Bit 4-7: Port type
    Bit 8-15: Port Number*/
    u16 general_info; 
    u32 max_lba_lo;
    u32 max_lba_hi;
    hba_port_t* hba_port;
} __attribute__((packed)) ahci_dev_t;

typedef struct ahci_controller_t
{
    hba_memory_t* abar;
    /* For legacy IRQ/IOAPIC mode, it's an IRQ line
    For MSI/MSI-X mode, it's the whole vector itself. */
    u8 interrupt_num;
    u32 device_count;
    ahci_dev_t* devices;
} ahci_controller_t;

typedef struct ahci_ioctl_rw_t
{
    u64 lba;
    u16 sector_count;
    void* io_buffer_base;
} ahci_ioctl_rw_t;

typedef enum
{
    AHCI_IOCTL_READ,
    AHCI_IOCTL_WRITE,
    AHCI_IOCTL_IDENTIFY,
    AHCI_IOCTL_PLACEHOLDER=0xFF
} ahci_ioctl_ops_t;