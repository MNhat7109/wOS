#include "fat.h"
#include "../disk/disk.h"
#include "../stdio.h"

typedef struct
{
    u8 jmp_ins[3];
    u8 oem_id[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 fat_count;
    u16 root_dir_entries; // In FAT32, this is useless
    u16 total_sectors; // Again, useless
    u8 media_type;
    u16 fat1x_sectors_per_fat; // Useless field in FAT32
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 large_sector_count;

    u32 fat32_sectors_per_fat;
    u16 flags;
    u16 fat_version;
    u32 root_cluster;
    u16 fsinfo_sector;
    u16 backup_boot_sector;
    u8 _reserved[12];
    u8 drive_number;
    u8 nt_flags;
    u8 serial_number[4];
    u8 volume_label[11];
    u8 sys_id[8];

} __attribute__((packed)) FAT32_bpb_t;

typedef struct
{
    u32 leading_sig;
    u8 _reserved0[480];
    u32 mid_sig;
    u32 lkf_cluster; // Last known free cluster
    u32 starting_cluster;
    u8 _reserved1[12];
    u32 trailing_sig;
} __attribute__((packed)) FAT32_fsinfo_t;

volatile struct FAT_data_t
{
    union
    {
        FAT32_bpb_t boot_sector;
        u8 boot_sector_buffer[0x200];
    };
    union
    {
        FAT32_fsinfo_t fsi_sector;
        u8 fsi_sector_buffer[0x200];
    };

} *data_pack = (struct FAT_data_t*)0x1A000; 
// Put it there bc I forgor the memory map 💀
// A downside of manually allocating heap memory, but bro,
// who tf set up malloc() and free() at this point?

u8* FAT_cache = (u8*)0x8200; // We reuses the FAT cache from stage1

bool FAT_read_boot_sector()
{
    return disk_read_sectors(0, 1, data_pack->boot_sector_buffer);
}

bool FAT_read_fsinfo_sector()
{
    return disk_read_sectors(data_pack->boot_sector.fsinfo_sector
        , 1, data_pack->fsi_sector_buffer);
}

bool FAT_fsinfo_validity_check()
{
    if (data_pack->fsi_sector.leading_sig!=0x41615252
        || data_pack->fsi_sector.mid_sig!=0x61417272
        || data_pack->fsi_sector.trailing_sig!=0xAA550000)
        return false;
    return true;
}

bool FAT_read_cache(u32 position)
{
    if (position>=data_pack->boot_sector.fat_count*
        data_pack->boot_sector.fat32_sectors_per_fat)
    {
        kprintf("FAT: FAT cache read out of range\n");
        return false;
    }
    return disk_read_sectors(
        data_pack->boot_sector.reserved_sectors+position,
        1, FAT_cache);
}

bool FAT_init()
{
    kprintf("FAT: ATA drive no. %u, offset=0x%x\n", 
        disk_packet->ata_drive_number, disk_packet->partition_offset);

    if (!FAT_read_boot_sector())
    {
        kprintf("FAT: cannot read boot sector\n");
        return false;
    }

    if (!FAT_read_fsinfo_sector())
    {
        kprintf("FAT: cannot read FSInfo sector\n");
        return false;
    }

    if (!FAT_fsinfo_validity_check())
    {
        kprintf("FAT: Invalid FSInfo sector\n");
        return false;
    }


    return true;
}

u32 FAT_next_cluster(u32 current_cluster)
{
    u32 offset = current_cluster*4;
    u32 pos = offset/data_pack->boot_sector.bytes_per_sector;
    u32 idx = offset%data_pack->boot_sector.bytes_per_sector;
    if (!FAT_read_cache(pos)) return 0;
    u32 value = *(u32*)&FAT_cache[idx] & 0x0FFFFFFF;

    return value;
}