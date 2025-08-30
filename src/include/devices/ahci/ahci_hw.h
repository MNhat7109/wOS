#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_ERR     0x01    // Error

#define AHCI_SIG_ATA   0x00000101
#define AHCI_SIG_ATAPI 0xEB140101
#define AHCI_SIG_SEMB  0xC33C0101
#define AHCI_SIG_PM    0x96690101

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

#define FIS_TYPE_REG_H2D   0x27
#define FIS_TYPE_REG_D2H   0x34
#define FIS_TYPE_DMA_ACT   0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA      0x46
#define FIS_TYPE_BIST      0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS  0xA1

typedef struct hba_port_t
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

typedef struct hba_memory_t
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

// HBA

void hba_setup_cmd_hdr(hba_cmd_hdr_t* cmd_hdr, int slot, u32 fis_length, u32 prdt_length, u32 direction_write);
void hba_setup_prdt(hba_cmd_table_t* cmd_table, int index, void* buffer, u32 byte_count);
void hba_reset(hba_memory_t* abar);
bool hba_perform_bios_os_handoff(hba_memory_t* abar);

bool hba_port_fis_start(hba_port_t* port);
bool hba_port_fis_stop(hba_port_t* port);
bool hba_port_cmd_start(hba_port_t* port);
bool hba_port_cmd_stop(hba_port_t* port);

bool hba_port_shutdown(hba_port_t* port);
bool hba_port_startup(hba_port_t* port);

bool hba_port_reset(hba_port_t* port);

u32 hba_port_type_detect(hba_port_t* port);

// FIS

void fis_reg_h2d_setup(fis_reg_h2d_t* cmd_fis, u8 command, u64 lba, u32 count, bool lba48);


// CMD setup

bool ahci_prepare_cmd(hba_port_t* port, int slot, u8 cmd, u16 count, bool use_lba, bool write, u64 lba, void* buffer);
bool ahci_wait_tfd(hba_port_t* port);