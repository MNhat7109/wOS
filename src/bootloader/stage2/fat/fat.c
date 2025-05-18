#include "fat.h"
#include "../disk/disk.h"
#include "../stdio.h"
#include "../string/string.h"
#include "../ctype/ctype.h"

#define SECTOR_SIZE 0x200
#define ROOT_DIR_HANDLE -1

#define ENTRY_ATTR_RO     0x01
#define ENTRY_ATTR_HIDDEN 0x02
#define ENTRY_ATTR_SYS    0x04
#define ENTRY_ATTR_VOL_ID 0x08
#define ENTRY_ATTR_DIR    0x10
#define ENTRY_ATTR_ARCHVE 0x20
#define ENTRY_ATTR_LFN    ENTRY_ATTR_RO | ENTRY_ATTR_HIDDEN \
| ENTRY_ATTR_SYS | ENTRY_ATTR_VOL_ID


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

typedef struct
{
    u8 buffer[SECTOR_SIZE];
    FAT_file_t public;
    bool opened;
    u32 first_cluster;
    u32 current_cluster;
    u32 current_sector_in_cluster;
} FAT_file_data_t;

volatile struct FAT_data_t
{
    union
    {
        FAT32_bpb_t boot_sector;
        u8 boot_sector_buffer[SECTOR_SIZE];
    };
    union
    {
        FAT32_fsinfo_t fsi_sector;
        u8 fsi_sector_buffer[SECTOR_SIZE];
    };

    FAT_file_data_t root_dir;
    FAT_file_data_t opened_files[10];
} *data_pack = (struct FAT_data_t*)0x10F00; 
// Put it there bc I forgor the memory map 💀
// A downside of manually allocating heap memory, but bro,
// who tf set up malloc() and free() at this point?

u32 first_data_sector;

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

u32 FAT_cluster_to_lba(u32 cluster)
{
    return first_data_sector + (cluster-2)*data_pack->boot_sector.sectors_per_cluster;
}

bool FAT_init(u8 ata_drive_number, void* partition_offset)
{
    if (!disk_init(ata_drive_number, partition_offset))
    {
        kprintf("FAT: Disk init error, FAT init cannot continue.\n");
        return false;
    }
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

    first_data_sector = data_pack->boot_sector.reserved_sectors 
    + data_pack->boot_sector.fat_count*data_pack->boot_sector.fat32_sectors_per_fat;

    // Set some starting vars for root directory
    data_pack->root_dir.public.file_handle_no = ROOT_DIR_HANDLE;
    data_pack->root_dir.public.is_directory = true;
    data_pack->root_dir.public.file_pos = 0;
    data_pack->root_dir.public.file_size = 0; // Can say the root directory size is infinity (to the end of partition, ofc)
    data_pack->root_dir.opened = true;
    data_pack->root_dir.first_cluster = data_pack->boot_sector.root_cluster;
    data_pack->root_dir.current_cluster = data_pack->root_dir.first_cluster;
    data_pack->root_dir.current_sector_in_cluster = 0;

    // Now read the starting buffer
    if (!disk_read_sectors(FAT_cluster_to_lba(data_pack->root_dir.current_cluster), 1, data_pack->root_dir.buffer))
    {
        kprintf("FAT: Error reading root directory buffer\n");
        return false;
    }
    
    for (int i=0;i<10;i++)
        data_pack->opened_files[i].opened = false;
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

u32 FAT_read(FAT_file_t* file, u32 count, void* buffer)
{
    // Get file data
    FAT_file_data_t* fd = (file->file_handle_no == ROOT_DIR_HANDLE)?
    &data_pack->root_dir:&data_pack->opened_files[file->file_handle_no];
    fd->public = *file;

    u8* blk = (u8*)buffer;

    if (!fd->public.is_directory || (fd->public.is_directory && fd->public.file_size != 0))
    {
        count = (count < fd->public.file_size-fd->public.file_pos)?count:fd->public.file_size-fd->public.file_pos;
    }
    
    while (count > 0)
    {
        // Get the bytes left in buffer, and the number of bytes to take out.
        u32 bytes_left = SECTOR_SIZE - (fd->public.file_pos%SECTOR_SIZE);
        u32 take = (count < bytes_left)?count:bytes_left;
        
        // Copy data from file data's buffer to our buffer, with an amount of take.
        // take == count: Copy to our buffer from the file's buffer, position at left_in_buffer, with an 
        // amount of take.
        // take == left: Copy all of the file's buffer, then update new data for the buffer.
        memcpy(blk, fd->buffer+(fd->public.file_pos%SECTOR_SIZE), take);
        blk+=take;
        fd->public.file_pos+=take;
        count -= take;
        
        if (take != bytes_left) continue;
        
        if (++fd->current_sector_in_cluster >= data_pack->boot_sector.sectors_per_cluster)
        {
            fd->current_sector_in_cluster = 0;
            fd->current_cluster = FAT_next_cluster(fd->current_cluster);
        }

        if (fd->current_cluster >= 0x0FFFFFF8) // End of cluster chain
        {
            fd->public.file_size = fd->public.file_pos;
            break;
        }

        // Read the sector
        if (!disk_read_sectors(FAT_cluster_to_lba(fd->current_cluster) + fd->current_sector_in_cluster
        , 1, fd->buffer))
        {
            kprintf("FAT: Error reading sector to buffer!\n");
            break;
        }
    }

    return blk-(u8*)buffer;
}

void FAT_get_short_name(const char* name, char out[12])
{
    memset(out, ' ', 12); out[11]='\0';
    
    const char* ext = strchr(name, '.'); // Example: test.txt -> ext = .txt
    if (!ext) ext = name+11;

    int cnter=0;
    for (;cnter<8 && name[cnter] && name+cnter<ext;cnter++)
        out[cnter] = toupper(name[cnter]);
    if ((ext!=name+11&&ext-name>8) || (ext==name+11&&ext-name>11))
    {
        out[6]='~'; out[7]='1'; // Deal with LFNs, just in case
    }
    if (ext == name+11) return;
    for (int i=0;i<3&&ext[i+1];i++)
        out[i+8] = toupper(ext[i+1]);

}

bool FAT_read_entry(FAT_file_t* file, void* entry)
{
    return FAT_read(file, sizeof(FAT_dir_entry), entry) == sizeof(FAT_dir_entry);
}

bool FAT_find_file_entry(FAT_file_t* file, const char* name, FAT_dir_entry* entry)
{
    union {FAT_dir_entry standard; FAT_lfn_t lfn;} entry_in;
    char fat_name[12]; FAT_get_short_name(name, fat_name);
    while (FAT_read_entry(file, (void*)&entry_in))
    {
        // Skip over LFNs completely
        if (entry_in.lfn.attribute == 0x0F)
        continue;

        if (memcmp(fat_name, entry_in.standard.short_name, 11) == 0)
        {
            *entry = entry_in.standard;
            return true;
        }
    }
    return false;
}

FAT_file_t* FAT_open_file_entry(FAT_dir_entry* entry)
{
    int handle =-2;
    for (int i=0;i<10;i++)
    {
        if (!data_pack->opened_files[i].opened)
            handle=i;
    }
    if (handle<0)
    {
        // Cannot find any free handles
        kprintf("FAT: Out of file handles\n");
        return NULL;
    }

    // Set up vars
    FAT_file_data_t* fd = &data_pack->opened_files[handle];
    fd->opened = true;
    fd->public.file_handle_no = handle;
    fd->public.is_directory = entry->attributes & ENTRY_ATTR_DIR;
    fd->public.file_pos = 0;
    fd->public.file_size = entry->file_size;
    fd->first_cluster = (entry->first_cluster_hi<<16) | (entry->first_cluster_lo);
    fd->current_cluster = fd->first_cluster;
    fd->current_sector_in_cluster = 0;

    if (!disk_read_sectors(FAT_cluster_to_lba(fd->current_cluster), 1, fd->buffer))
    {
        kprintf("FAT: Error reading file buffer\n");
        return NULL;
    }

    return &fd->public;
}

FAT_file_t* FAT_open(const char* path)
{
    char name[128];
    // Parse the file path.
    if (path[0] == '/') path++;
    FAT_file_t* file = &data_pack->root_dir.public; // Always search from the root directory first.
    while (*path)
    {
        bool last = false;
        FAT_dir_entry entry;
        const char* delim = strchr(path, '/');
        if (delim)
        {
            memcpy(name, path, delim-path);
            name[delim-path] = '\0';
            path = delim+1;
        }
        else // Scenario where we reach the end file/dir of the file path.
        {
            u32 len = strlen(path);
            memcpy(name, path, len);
            name[len+1]='\0';
            path+=len;
            last=true;
        }

        // Find dir entry
        bool status = FAT_find_file_entry(file, name, &entry);
        FAT_close(file);
        
        // Check if the entry is found
        if (!status)
        {
            kprintf("FAT: %s not found\n", name);
            return NULL;
        }

        // Check if entry is a dir, if the path has not reached last
        if (!last && !(entry.attributes & ENTRY_ATTR_DIR))
        {
            kprintf("FAT: %s is not a directory\n", name);
            return NULL;
        }

        file = FAT_open_file_entry(&entry);
    }

    return file;
}

void FAT_close(FAT_file_t* file)
{
    if (file->file_handle_no == ROOT_DIR_HANDLE)
    {
        file->file_pos = 0;
        data_pack->root_dir.current_cluster = data_pack->root_dir.first_cluster;
    }
    else
    {
        data_pack->opened_files[file->file_handle_no].opened = false;
    }
}

void FAT_seek(FAT_file_t* file, u32 pos)
{
    FAT_file_data_t* fd = file->file_handle_no==ROOT_DIR_HANDLE?
    &data_pack->root_dir:&data_pack->opened_files[file->file_handle_no];

    fd->public.file_pos = pos;
    kprintf("%u\n", fd->public.file_pos);
    u32 sect_num=fd->public.file_pos / SECTOR_SIZE;

    while (sect_num--)
    {
        if (++fd->current_sector_in_cluster >= data_pack->boot_sector.sectors_per_cluster)
        {
            fd->current_sector_in_cluster = 0;
            fd->current_cluster = FAT_next_cluster(fd->current_cluster);
        }
    
        if (fd->current_cluster >= 0x0FFFFFF8) // End of cluster chain
        {
            fd->public.file_size = fd->public.file_pos;
            return;
        }
    
        // Read the sector
        if (!disk_read_sectors(FAT_cluster_to_lba(fd->current_cluster) + fd->current_sector_in_cluster
        , 1, fd->buffer))
        {
            kprintf("FAT: Error reading sector to buffer!\n");
            return;
        }
    }
}