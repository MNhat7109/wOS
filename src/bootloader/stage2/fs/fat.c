#include "fs.h"
#include "fat.h"
#include "disk.h"
#include "partition.h"
#include "../stdio.h"
#include "../string/string.h"
#include "../containers/stack.h"
#include "../errno.h"

#define SECTOR_SIZE 0x200
#define FAT_CACHE_SIZE 5
#define FAT_CACHE_VALUE_UNKNOWN 0xFFFFFFFF
#define ROOT_DIR_HANDLE -1
#define MAX_HANDLES 16
#define DIR_ENTRY_SIZE 32

typedef struct fat_comp_bpb_t fat_comp_bpb_t;
typedef struct fat32_ebpb_t fat32_ebpb_t;
typedef struct fat32_fsinfo_t fat32_fsinfo_t;
typedef struct fat_standard_entry_t fat_standard_entry_t;
typedef struct fat_lfn_entry_t fat_lfn_entry_t;
typedef union fat_entry_t fat_entry_t;

extern const char* const str_media[];
extern const char* const str_ctler[];

static const char* const str_fat_ver[] = {
    "unknown",
    "fat12",
    "fat16",
    "fat32",
};

typedef enum
{
    FAT_UNKNOWN,
    FAT12,
    FAT16,
    FAT32
} fat_version_t;

typedef struct fat_comp_bpb_t
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
} __attribute__((packed)) fat_comp_bpb_t;

typedef struct fat1x_ebpb_t
{
    u8 drive_number;
    u8 flags;
    u8 signature;
    u8 serial_number[4];
    u8 volume_label[11];
    u8 sys_id[8];
} __attribute__((packed)) fat1x_ebpb_t;

typedef struct fat32_ebpb_t
{
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
} __attribute__((packed)) fat32_ebpb_t;

typedef struct fat32_fsinfo_t
{
    u32 leading_sig;
    u8 _reserved0[480];
    u32 mid_sig;
    u32 lkf_cluster; // Last known free cluster
    u32 starting_cluster;
    u8 _reserved1[12];
    u32 trailing_sig;
} fat32_fsinfo_t;

typedef struct fat_file_handle_t
{
    file_handle_t handle;
    u8 fat_file_buffer[SECTOR_SIZE];
} fat_file_handle_t;

static struct
{
    int fat_version;
    union
    {
        struct
        {
            fat_comp_bpb_t fat_comp_hdr; // FAT compatibility header between versions 
            union
            {
                fat1x_ebpb_t fat1x;
                fat32_ebpb_t fat32;
            };
        } __attribute__((packed)) fields;
        u8 buffer[SECTOR_SIZE];
    } boot_sector;

    union
    {
        fat32_fsinfo_t fat32;
        u8 buffer[SECTOR_SIZE];
    } fsinfo_sector;

    u8 fat_cache[FAT_CACHE_SIZE*SECTOR_SIZE];
    u32 fat_cache_pos;
    u32 data_section_lba;
    
    fat_file_handle_t file_handles[MAX_HANDLES];
    fat_file_handle_t root_dir_handle;
} fat_partition_data;

u32 fat_get_sectors_per_fat();
u32 fat_get_total_sectors();
u32 fat_get_end_of_cluster();
u32 fat_get_first_data_sector();
int fat_check_unknown_fs();
int fat_get_version();
int fat_populate_handle(fs_t* fs, fat_file_handle_t* fat_handle, file_handle_t* file_handle_in, file_handle_t** file_handle_out);
int fat_find_free_handle(fat_file_handle_t** handle_out);

int fat_read_boot_sector(disk_t* disk, partition_t* part);
int fat_read_cache(disk_t* disk, partition_t* part, u32 sector_pos);
int fat_read_fsinfo_sector(disk_t* disk, partition_t* part);
int fat_check_fsinfo();
u32 fat_cluster_to_lba(u32 cluster);
u32 fat_next_cluster(file_t* file_in, u32 current_cluster);
int fat_next_sector(file_t* file_in, fat_file_handle_t* fat_handle);

int fat_read_dir_entry(file_t* file, fat_entry_t* entry_out);
int fat_search_entry(file_t* file, const char* entry_name, fat_entry_t* entry_out);
void fat_register_entry(fs_t* fs, fat_standard_entry_t* entry, file_t** file_out);
void fat_traverse_dir_entry(file_t* file_in);

int fat_init(fs_t* fs);
u32 fat_read(file_t* file_in, u32 count, void* buffer);
int fat_open(fs_t* fs, const char* path, file_t** file_out);
void fat_close(file_t* file);
int fat_seek(file_t* file_in, u32 pos);
int fat_traverse(file_t* file_in);

const fs_ops_t fat_fs_ops = {
    .init = &fat_init,
    .read = &fat_read,
    .open = &fat_open,
    .close = &fat_close,
    .seek = &fat_seek,
    .traverse = &fat_traverse
};

const fs_ops_t* fat_load_ops()
{
    return &fat_fs_ops;
}

int fat_init(fs_t* fs)
{
    int status;
    status = fat_read_boot_sector(fs->disk, fs->partition);
    if (status < 0) return status;

    // Determine the FAT version
    fat_partition_data.fat_version = fat_get_version();

    // Calculate start LBA of data section
    fat_partition_data.data_section_lba = fat_get_first_data_sector();

    if (fat_partition_data.fat_version != FAT32) goto next;

    status = fat_read_fsinfo_sector(fs->disk, fs->partition);
    if (status <0) return status;

    status = fat_check_fsinfo();
    if (status < 0) return status;

    fat_partition_data.fat_cache_pos = FAT_CACHE_VALUE_UNKNOWN;
next:
    u32 total_sectors = fat_get_total_sectors();
    u32 sectors_per_fat = fat_get_sectors_per_fat();
    u32 root_dir_sectors = (
        fat_partition_data.boot_sector.fields.fat_comp_hdr.root_dir_entries*32
        + (fat_partition_data.boot_sector.fields.fat_comp_hdr.
    bytes_per_sector-1)
    ) / fat_partition_data.boot_sector.fields.fat_comp_hdr.
    bytes_per_sector;

    u32 root_cluster = (fat_partition_data.fat_version == FAT32)?
    fat_partition_data.boot_sector.fields.fat32.root_cluster:
    root_dir_sectors / fat_partition_data.boot_sector.fields.fat_comp_hdr.
    sectors_per_cluster;

    file_handle_t root_handle = (file_handle_t)
    {
            .public = (file_t){
                .handle = ROOT_DIR_HANDLE,
                .is_dir = 1,
                .file_pos = 0,
                .file_size = 0,
                .fs = fs
            },
            .opened = 1,
            .first_block = root_cluster,
            .current_block = root_cluster,
            .block_size = fat_partition_data.boot_sector.fields.fat_comp_hdr.
            bytes_per_sector,
            .current_pos_in_block = 0
    };

    status = fat_populate_handle(fs, &fat_partition_data.root_dir_handle, &root_handle, NULL);
    if (status < 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Failed to populate root directory handle\n");
        return status;
    }

    for (u32 i=0;i<MAX_HANDLES;i++) 
    fat_partition_data.file_handles[i].handle.opened = 0;

    kdebugf(DEBUG_INFO, MODULE_FS, "On partition %s%up%u:\n",
    str_media[fs->disk->media_type], fs->disk->pos, fs->partition->attributes.pos);
    kdebugf(DEBUG_INFO, MODULE_FS, "Filesystem found: %s\n", 
    str_fat_ver[fat_partition_data.fat_version]);
    kdebugf(DEBUG_INFO, MODULE_FS, "Additional info:\n"
    "\tTotal sectors: %u\n""\tReserved sector count: %u\n""\tFAT count: %u\n"
    "\tSectors per FAT:%u\n""\tSectors per cluster: %u\n"
    "\tData section start at sector %u\n",
    total_sectors,
    fat_partition_data.boot_sector.fields.fat_comp_hdr.reserved_sectors,
    fat_partition_data.boot_sector.fields.fat_comp_hdr.fat_count,
    sectors_per_fat,
    fat_partition_data.boot_sector.fields.fat_comp_hdr.sectors_per_cluster,
    fat_partition_data.data_section_lba);

    return 0;
}

u32 fat_read(file_t* file_in, u32 count, void* buffer)
{
    if (!buffer) return 0;
    if (fat_check_unknown_fs() < 0) return 0;

    u8* orig_buffer = (u8*)buffer;

    fat_file_handle_t* fhp = (file_in->handle == ROOT_DIR_HANDLE)?
    &fat_partition_data.root_dir_handle:
    &fat_partition_data.file_handles[file_in->handle];

    if (!fhp->handle.public.is_dir || (fhp->handle.public.is_dir && fhp->handle.public.file_size !=0))
        count = (count < fhp->handle.public.file_size-fhp->handle.public.file_pos)?
        count:
        fhp->handle.public.file_size-fhp->handle.public.file_pos;

    while (count > 0)
    {
        // Get the bytes left in buffer, and the number of bytes to take out.
        u32 bytes_left = SECTOR_SIZE - (fhp->handle.public.file_pos%SECTOR_SIZE);
        u32 bytes_to_read = (count < bytes_left)?count:bytes_left;

        // Copy data from file data's buffer to our buffer, with a size of (bytes_to_read).
        // (bytes_to_read) == count: Copy to our buffer from the file's buffer, position at left_in_buffer, with a 
        // size of (bytes_to_read).
        // (bytes_to_read) == left: Copy all of the file's buffer, then update new data for the buffer.

        memcpy(buffer, fhp->fat_file_buffer+(fhp->handle.public.file_pos%SECTOR_SIZE), bytes_to_read);
        buffer = (u8*)buffer+bytes_to_read;
        fhp->handle.public.file_pos+=bytes_to_read;
        count -= bytes_to_read;

        if (bytes_to_read != bytes_left) continue;

        int status = fat_next_sector(file_in, fhp);
        if (status != 0) break;
    }

    return (u8*)buffer-orig_buffer;
}

extern char temp_name[];
int fat_open(fs_t* fs, const char* path, file_t** file_out)
{
    if (!path) return -1;
    char delim = '/';
    if (path[0] == delim) path++;
    u8 last_in_path = 0;
    file_t* fd = &fat_partition_data.root_dir_handle.handle.public;

    while (*path)
    {
        u8 found_entry[DIR_ENTRY_SIZE];
        const char* delim_pos = strchr(path, delim);
        if (delim_pos)
        {
            u32 entry_len = delim_pos-path;
            if (entry_len == 0) continue;
            memcpy(temp_name, path, entry_len);
            temp_name[entry_len] = '\0';
            path=delim_pos+1;
        }
        else
        {
            u32 path_len = strlen(path);
            memcpy(temp_name, path, path_len);
            temp_name[path_len+1]='\0';
            path+=path_len;
            last_in_path = 1;
        }

        int status = fat_search_entry(fd, temp_name, (fat_entry_t*)found_entry);
        if (status < 0) return -1;

        fat_register_entry(fs, (fat_standard_entry_t*)found_entry, &fd);
    }

    *file_out = fd;
    return 0;
}

void fat_close(file_t* file)
{
    if (file->handle == ROOT_DIR_HANDLE)
    {
        file->file_pos = 0;
        fat_partition_data.file_handles[file->handle].handle.current_block =
        fat_partition_data.file_handles[file->handle].handle.first_block;
    }
    else
    {
        fat_partition_data.file_handles[file->handle].handle.opened = 0;
    }
}

int fat_seek(file_t* file_in, u32 pos)
{
    fat_file_handle_t* fhp = (file_in->handle == ROOT_DIR_HANDLE)?
    &fat_partition_data.root_dir_handle:
    &fat_partition_data.file_handles[file_in->handle];

    u32 sector_pos = pos / SECTOR_SIZE;

    fhp->handle.current_block = fhp->handle.first_block;
    while (sector_pos--)
    {
        int status = fat_next_sector(file_in, fhp);
        if (status > 0) return -status;
        if (status < 0) break;
    }

    fhp->handle.public.file_pos = pos;
    return 0;
}

int fat_traverse(file_t* file_in)
{
    if (!file_in) return -1;
    fat_traverse_dir_entry(file_in);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////

int fat_read_boot_sector(disk_t* disk, partition_t* part)
{
    int status = partition_read(disk, part, 0, 1, fat_partition_data.boot_sector.buffer);
    if (status < 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Failed to read boot sector\n");
    }

    return status;
}

int fat_read_cache(disk_t* disk, partition_t* part, u32 sector_pos)
{
    u32 spf = fat_get_sectors_per_fat();
    u32 fat_sector_count = fat_partition_data.boot_sector.fields.fat_comp_hdr.
    fat_count * spf;
    if (sector_pos >= fat_sector_count)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "FAT cache sector_position out of range\n");
        return -EOOB;
    }

    u32 start_of_fat = fat_partition_data.boot_sector.fields.fat_comp_hdr.
    reserved_sectors;

    int status = partition_read(
        disk, 
        part, 
        start_of_fat+sector_pos, 
        FAT_CACHE_SIZE, 
        fat_partition_data.fat_cache
    );
    if (status < 0)
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Failed to read FAT cache\n");
    
    return status;
}

int fat_read_fsinfo_sector(disk_t* disk, partition_t* part)
{
    int status = partition_read(
        disk, 
        part, 
        fat_partition_data.boot_sector.fields.fat32.fsinfo_sector, 
        1, 
        fat_partition_data.fsinfo_sector.buffer
    );
    if (status < 0)
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Failed to read FSInfo sector for FAT32\n");
    
    return status;
}

int fat_check_fsinfo()
{
    // This is haram.
    return (
        fat_partition_data.fsinfo_sector.fat32.leading_sig == 0x41615252
        && fat_partition_data.fsinfo_sector.fat32.mid_sig == 0x61417272
        && fat_partition_data.fsinfo_sector.fat32.trailing_sig == 0xAA550000
    ) - 1; // Well, if the criteria aren't met, then it'll return 0-1 = -1, 1-1 = 0 otherwise.
}

///////////////////////////////////////////////////////////////////////////////////////////////

u32 fat_get_sectors_per_fat()
{
    return (fat_partition_data.
            boot_sector.
            fields.
            fat_comp_hdr.
            fat1x_sectors_per_fat == 0)?
            fat_partition_data.
            boot_sector.
            fields.
            fat32.
            fat32_sectors_per_fat:
            fat_partition_data.
            boot_sector.
            fields.
            fat_comp_hdr.
            fat1x_sectors_per_fat;
}

u32 fat_get_total_sectors()
{
    return (fat_partition_data.boot_sector.fields.fat_comp_hdr.total_sectors == 0)?
    fat_partition_data.boot_sector.fields.fat_comp_hdr.large_sector_count:
    fat_partition_data.boot_sector.fields.fat_comp_hdr.total_sectors;
}

u32 fat_get_end_of_cluster()
{
    u32 eoc;
    switch (fat_partition_data.fat_version)
    {
        case FAT12:
            eoc = 0xFF8;
            break;
        case FAT16:
            eoc = 0xFFF8;
            break;
        case FAT32:
            eoc = 0x0FFFFFF8;
            break;
        default:
            eoc = 0;
            break;
    }

    return eoc;
}

u32 fat_cluster_to_lba(u32 cluster)
{
    return fat_partition_data.data_section_lba+(cluster-2)*
    fat_partition_data.boot_sector.fields.fat_comp_hdr.sectors_per_cluster;
}

u32 fat_get_first_data_sector()
{
    u32 data_section_lba;
    // In FAT32, the root directory location is not fixed. As a result, field root_dir_entries
    // is 0, making root_dir_sectors holds 0 as a value;
    u32 root_dir_sectors = (
        fat_partition_data.boot_sector.fields.fat_comp_hdr.root_dir_entries*32
        + (fat_partition_data.boot_sector.fields.fat_comp_hdr.
    bytes_per_sector-1)
    ) / fat_partition_data.boot_sector.fields.fat_comp_hdr.
    bytes_per_sector;

    // Data section starts after the FAT region for FAT32 (because there's no root directory region)
    // and after the root directory region for earlier versions of FAT
    u32 sectors_per_fat = fat_get_sectors_per_fat();
    data_section_lba = 
    (
        fat_partition_data.boot_sector.fields.fat_comp_hdr.reserved_sectors +
        (
            fat_partition_data.boot_sector.fields.fat_comp_hdr.fat_count *
            sectors_per_fat
        )
    ) + root_dir_sectors;
    return data_section_lba;
}

int fat_get_version()
{
    int fat_version;
    u32 total_sectors = fat_get_total_sectors();
    u32 data_section_lba = fat_get_first_data_sector();

    u32 data_sector_count = total_sectors-data_section_lba;
    u32 total_data_clusters = data_sector_count / fat_partition_data.boot_sector.
    fields.fat_comp_hdr.sectors_per_cluster;

    if (total_data_clusters < 4085) fat_version = FAT12;
    else if (total_data_clusters < 65525) fat_version = FAT16;
    else fat_version = FAT32;
    return fat_version;
}

int fat_check_unknown_fs()
{
    return (
        fat_partition_data.fat_version != FAT_UNKNOWN &&
        *(u16*)(fat_partition_data.boot_sector.buffer+510) == 0xAA55
    ) - 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////

u32 fat_next_cluster(file_t* file_in, u32 current_cluster)
{
    if (!file_in || !file_in->fs) return FAT_CACHE_VALUE_UNKNOWN;
    u32 offset=0, value = 0;
    switch (fat_partition_data.fat_version)
    {
        case FAT12:
            offset = current_cluster*3/2;
            break;
        case FAT16:
            offset = current_cluster*2;
            break;
        case FAT32:
            offset = current_cluster*4;
            break;
    }

    u32 sector_pos = offset/SECTOR_SIZE;
    
    if (
        sector_pos < fat_partition_data.fat_cache_pos ||
        sector_pos >= fat_partition_data.fat_cache_pos +FAT_CACHE_SIZE)
    {
        int status = fat_read_cache(file_in->fs->disk, file_in->fs->partition, sector_pos);
        if (status < 0) return FAT_CACHE_VALUE_UNKNOWN;
        fat_partition_data.fat_cache_pos=sector_pos;
    }
    
    offset -= (fat_partition_data.fat_cache_pos*SECTOR_SIZE);

    switch (fat_partition_data.fat_version)
    {
        case FAT12:
            value = (current_cluster & 1)?
            (*(u16*)(fat_partition_data.fat_cache+offset)) >> 4:
            (*(u16*)(fat_partition_data.fat_cache+offset)) & 0x0FFF;
            break;
        case FAT16:
            value = (*(u16*)(fat_partition_data.fat_cache+offset)) & 0xFFFF;
            break;
        case FAT32:
            value = (*(u32*)(fat_partition_data.fat_cache+offset)) & 0x0FFFFFFF;
            break;
    }
    return value;
}

int fat_populate_handle(fs_t* fs, fat_file_handle_t* fat_handle, file_handle_t* file_handle_in, file_handle_t** file_handle_out)
{
    if (!fat_handle || !file_handle_in) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Bug potential. Check fat_handle or file_handle_in address values\n");
        return -EINVAL;
    }

    *fat_handle = (fat_file_handle_t){
        .handle = *file_handle_in
    };


    int status = partition_read(
        fs->disk, fs->partition,
        fat_cluster_to_lba(fat_handle->handle.current_block),
        1,
        fat_handle->fat_file_buffer
    );
    if (status < 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Failed to read to buffer at handle no. %d\n", 
            fat_handle->handle.public.handle
        );
        return status;
    }

    if (file_handle_out)
    *file_handle_out = &fat_handle->handle;

    return 0;
}

int fat_find_free_handle(fat_file_handle_t** handle_out)
{
    for (int i=0;i<MAX_HANDLES; i++)
    {
        fat_file_handle_t* current_handle = &fat_partition_data.file_handles[i];
        if (current_handle->handle.opened) continue;
        *handle_out = current_handle;

        return i;
    }
    kdebugf(DEBUG_CRITICAL, MODULE_FS, "Out of free file handles\n");
    return -1;
}

int fat_next_sector(file_t* file_in, fat_file_handle_t* fat_handle)
{
    if (++fat_handle->handle.current_pos_in_block == 
        fat_partition_data.boot_sector.fields.fat_comp_hdr.sectors_per_cluster
    )
    {
        fat_handle->handle.current_pos_in_block = 0;
        u32 next_cluster = fat_next_cluster(file_in,
            fat_handle->handle.current_block
        );

        if (next_cluster >= fat_get_end_of_cluster())
        {
            kdebugf(DEBUG_INFO, MODULE_FS, "Reached EOF at cluster=%x\n",next_cluster);
            return -1;
        }

        fat_handle->handle.current_block = next_cluster;
    }

    // Fill in our buffer
    int status = partition_read(
        file_in->fs->disk, file_in->fs->partition,
        fat_cluster_to_lba(fat_handle->handle.current_block)+
        fat_handle->handle.current_pos_in_block,
        1, fat_handle->fat_file_buffer
    );
    if (status < 0)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_FS, "Error trying to filling up buffer at pos=0x%x, bs=%u, cluster=%u\n",
        fat_handle->handle.current_pos_in_block, fat_handle->handle.block_size, fat_handle->handle.current_block);
        return 1;
    }

    return 0;
}

char temp_name[256];