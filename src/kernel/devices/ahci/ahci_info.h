#pragma once
#include "../../stdint.h"
#include <stdbool.h>
#include "../driver.h"

#define HBA_GHC_AE (1<<31)
#define HBA_GHC_MRSM (1<<2)
#define HBA_GHC_IE (1<<1)
#define HBA_GHC_HR (1<<0)

#define HBA_BOHC_BB (1<<4)
#define HBA_BOHC_OOC (1<<3)
#define HBA_BOHC_SOOE (1<<2)
#define HBA_BOHC_OOS (1<<1)
#define HBA_BOHC_BOS (1<<0)

#define HBA_PxIS_CPDS  (1<<31) // Cold Port Detect Status
#define HBA_PxIS_TFES  (1<<30) // Task File Error Status
#define HBA_PxIS_HBFS  (1<<29) // Host Bus Fatal Error Status
#define HBA_PxIS_HBDS  (1<<28) // Host Bus Data Error Status
#define HBA_PxIS_IFS   (1<<27) // Interface Fatal Error Status
#define HBA_PxIS_INFS  (1<<26) // Interface Non-Fatal Error Status
#define HBA_PxIS_OFS   (1<<24) // Overflow Status
#define HBA_PxIS_IPMS  (1<<23) // Incorrect Port Multiplier Status
#define HBA_PxIS_PRCS  (1<<22) // PhyRdy Change Status
#define HBA_PxIS_DMPS  (1<<7)  // Device Mechanical Presence Status
#define HBA_PxIS_PCS   (1<<6)  // Port Connect Change Status
#define HBA_PxIS_DPS   (1<<5)  // Descriptor Processed Status
#define HBA_PxIS_UFS   (1<<4)  // Unknown FIS Interrupt Status
#define HBA_PxIS_SDBS  (1<<3)  // Set Device Bits Interrupt Status
#define HBA_PxIS_DSS   (1<<2)  // DMA Setup FIS Interrupt Status
#define HBA_PxIS_PSS   (1<<1)  // PIO Setup FIS Interrupt Status
#define HBA_PxIS_DHRS  (1<<0)  // Device to Host Register FIS Interrupt Status

#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRQ     0x08    // Data request ready

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
    u32 cmd_list_base_lo;    
    u32 cmd_list_base_hi;
    u32 fis_base_lo;    
    u32 fis_base_hi;
    u32 interrupt_status;
    u32 interrupt_enable;
    u32 command_status;
    u32 _reserved0;
    u32 task_file_data;
    u32 signature;    
    u32 sata_status;
    u32 sata_control;
    u32 sata_error;
    u32 sata_active;
    u32 cmd_issue;
    u32 sata_notify;
    u32 fis_switch_control;
    u32 _reserved1[11];
    u32 vendor[4];
} __attribute__((packed)) hba_port_t;

typedef struct
{
    u32 host_capability;
    u32 global_host_ctl;
    u32 interrupt_status;
    u32 port_implemented;
    u32 version;
    u32 com_cmplt_coalescing_ctl;
    u32 com_cmplt_coalescing_ports;
    u32 enclosure_mgmt_location;
    u32 enclosure_mgmt_ctl;
    u32 host_capability_ext;
    u32 bios_os_handoff_ctl_sts;
    u8 _reserved[0xA0-0x2C];
    u8 vendor[0x100-0xA0];
    volatile hba_port_t ports[1];
}  __attribute__((packed)) hba_memory_t;

typedef struct
{
    u8 cmd_fis_length : 5;
    
    u8 atapi : 1;
    u8 write : 1;
    u8 prefetch : 1;

    u8 reset : 1;
    u8 bist  : 1;
    u8 clear_busy : 1;
    u8 _reserved0 : 1;
    u8 port_multi_port : 4;

    u16 prdt_length;

    volatile u32 prd_byte_cnt;

    u32 cmd_table_base_lo;
    u32 cmd_table_base_hi;

    u32 _reserved1[4];

} __attribute__((packed)) hba_cmd_hdr_t;

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
    u32 data_base_addr_lo;
    u32 data_base_addr_hi;
    u32 _reserved0; 

    u32 data_byte_cnt : 22;
    u32 _reserved1 : 9;
    u32 interrupt : 1;
} __attribute__((packed)) hba_prdt_entry_t;

typedef struct
{
    u8 cmd_fis[64];
    u8 atapi_cmd[16];
    u8 _reserved[48];
    hba_prdt_entry_t prdt_entries[1];
} __attribute__((packed)) hba_cmd_table_t;

typedef struct
{
    bool used;
    bool lba_mode;
    bool is_lba48;
    bool status;
    bool cmd_done[32];
    int num;
    int type;
    hba_port_t* hba_port;
} __attribute__((packed)) ahci_port_t;

#define AHCI_MAX_PORT_ENTRIES 128
extern hba_memory_t* abar;
extern u32 abar_phys;
extern int current_port_count;
extern u8 interrupt_line;
extern int max_cmd_slot_cnt;
extern int max_port_cnt;
extern generic_driver_io_t ahci_io_pack;
extern ahci_port_t ports[AHCI_MAX_PORT_ENTRIES];
