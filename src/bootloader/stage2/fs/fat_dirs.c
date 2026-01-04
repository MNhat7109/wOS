#include "fat.h"
#include "fs.h"
#include "../string/string.h"
#include "../stdio.h"

#define ENTRY_ATTR_RO     0x01
#define ENTRY_ATTR_HIDDEN 0x02
#define ENTRY_ATTR_SYS    0x04
#define ENTRY_ATTR_VOL_ID 0x08
#define ENTRY_ATTR_DIR    0x10
#define ENTRY_ATTR_ARCHIVE 0x20
#define ENTRY_ATTR_LFN    ENTRY_ATTR_RO | ENTRY_ATTR_HIDDEN \
| ENTRY_ATTR_SYS | ENTRY_ATTR_VOL_ID

typedef struct fat_file_handle_t fat_file_handle_t;

typedef struct fat_standard_entry_t
{
    char short_name[11];
    u8 attributes;
    u8 _reserved;
    u8 creation_time_tenth;
    u16 creation_time;
    u16 creation_date;
    u16 last_accessed_date;
    u16 first_cluster_hi;
    u16 last_mod_time;
    u16 last_mod_date;
    u16 first_cluster_lo;
    u32 file_size;
} __attribute__((packed)) fat_standard_entry_t;

typedef struct fat_lfn_entry_t
{
    u8 order;
    u16 first_chunk[5];
    u8 attributes;
    u8 entry_type;
    u8 checksum;
    u16 next_chunk[6];
    u16 zero;
    u16 final_chunk[2];
} __attribute__((packed)) fat_lfn_entry_t;

typedef union fat_entry_t
{
    fat_standard_entry_t standard;
    fat_lfn_entry_t lfn;
} fat_entry_t;

u32 fat_next_cluster(disk_t* disk, partition_t* part, u32 current_cluster);

int fat_populate_handle(fs_t* fs, fat_file_handle_t* fat_handle, file_handle_t* file_handle_in, file_handle_t** file_handle_out);
int fat_find_free_handle(fat_file_handle_t** handle_out);

int fat_is_special_dir(fat_standard_entry_t* entry);

int fat_read_dir_entry(file_t* file, fat_entry_t* entry_out)
{
    return (
        file->fs &&
        file->fs->ops &&
        file->fs->ops->read &&
        file->fs->ops->read(file, sizeof(fat_entry_t), entry_out)
        == sizeof(fat_entry_t)
    ) -1;
}

int fat_search_entry(file_t* file, const char* entry_name, fat_entry_t* entry_out)
{
    fat_entry_t entry; u8 done_lfn_early=0, current_entry=0;
    u32 name_len = strlen(entry_name);

    while (fat_read_dir_entry(file, &entry) == 0)
    {
        if (entry.standard.short_name[0] == 0) break;
        // Parse LFN
        if (entry.lfn.attributes == 0x0F)
        {
            u8 is_last_entry = entry.lfn.order >> 4;
            current_entry = (entry.lfn.order & 0xF);

            if (is_last_entry)
            {
                done_lfn_early = 0;
            }

            if (done_lfn_early) continue;

            // Compare chars
            u32 offset = 13*(current_entry-1);
            if (is_last_entry && name_len <= offset) 
            {
                goto done_lfn;
            }

            for (int i=0;i<5;i++)
            {
                u16 entry_char = entry.lfn.first_chunk[i]; // TODO: UTF16 to UTF8
                if (entry_char == 0 || entry_char == 0xFFFF) goto final_check;
                if (entry_name[offset] != entry_char) goto done_lfn;
                offset++;
            }

            for (int i=0;i<6;i++)
            {
                u16 entry_char = entry.lfn.next_chunk[i]; // TODO: UTF16 to UTF8
                if (entry_char == 0 || entry_char == 0xFFFF) goto final_check;
                if (entry_name[offset] != entry_char) goto done_lfn;
                offset++;
            }

            for (int i=0;i<2;i++)
            {
                u16 entry_char = entry.lfn.final_chunk[i]; // TODO: UTF16 to UTF8
                if (entry_char == 0 || entry_char == 0xFFFF) goto final_check;
                if (entry_name[offset] != entry_char) goto done_lfn;
                offset++;
            }

            
            continue;
final_check:
            if (!is_last_entry || (is_last_entry && offset != name_len))
done_lfn:
            done_lfn_early=1;
        } 
                else if ((entry.standard.attributes&ENTRY_ATTR_VOL_ID) ||
    ((entry.standard.attributes&ENTRY_ATTR_SYS))) continue;
        // Parse standard entry
        else
        {
            if (done_lfn_early) continue;

            *entry_out = entry;
            return 0;
        }
    }

    kdebugf(DEBUG_CRITICAL, MODULE_FS, "%s not found\n", entry_name);
    return -1;
}

file_handle_t handle_value;
void fat_register_entry(fs_t* fs, fat_standard_entry_t* entry, file_t** file_out)
{
    fat_file_handle_t* fhp;
    int handle = fat_find_free_handle(&fhp);
    if (handle < 0) return;
    
    u32 file_cluster = ((u32)entry->first_cluster_lo) | ((u32)entry->first_cluster_hi<<16);

    handle_value = (file_handle_t){
        .opened = 1,
        .first_block = file_cluster,
        .current_block = file_cluster,
        .current_pos_in_block = 0,
        .block_size = 512,
        .public = {
            .fs = fs,
            .handle = handle,
            .file_pos = 0,
            .file_size = entry->file_size,
            .is_dir = entry->attributes & ENTRY_ATTR_DIR
        }
    };

    file_handle_t* file_handle_out;
    int status = fat_populate_handle(fs, fhp, &handle_value, &file_handle_out);
    if (status < 0)
    {
        return;
    }
    
    *file_out = &file_handle_out->public;
}

extern char dir_temp_name[];
void fat_traverse_dir_entry(file_t* file_in)
{
    if (!file_in) return;
    fat_entry_t entry; u8 skip_lfn = 0; u32 entry_count = 0;
    
    kprintf("Directory content:\n");
    while (fat_read_dir_entry(file_in, &entry) == 0)
    {
        if (entry.standard.short_name[0] == 0) break;
        if (entry.lfn.attributes == 0x0F)
        {
            if (skip_lfn) continue;
            u32 offset = 13*((entry.lfn.order&0xF)-1);
            u8 is_last_entry = entry.lfn.order >> 4;
            
            for (int i=0;i<5;i++)
            {
                // TODO
                if (entry.lfn.first_chunk[i] == 0xFFFF)
                    goto done;
                dir_temp_name[offset] = entry.lfn.first_chunk[i];
                if (entry.lfn.first_chunk[i] == 0) goto done;
                offset++;
            }

            for (int i=0;i<6;i++)
            {
                // TODO
                if (entry.lfn.next_chunk[i] == 0xFFFF)
                    goto done;
                dir_temp_name[offset] = entry.lfn.next_chunk[i];
                if (entry.lfn.next_chunk[i] == 0) goto done;
                offset++;
            }
            
            for (int i=0;i<2;i++)
            {
                // TODO
                if (entry.lfn.final_chunk[i] == 0xFFFF)
                    goto done;
                dir_temp_name[offset] = entry.lfn.final_chunk[i];
                if (entry.lfn.final_chunk[i] == 0) goto done;
                offset++;
            }
            continue;
done:
            if (!is_last_entry) skip_lfn = 1;
        }
        else if ((entry.standard.attributes&ENTRY_ATTR_VOL_ID) ||
    ((entry.standard.attributes&ENTRY_ATTR_SYS))) continue;
        else
        {
            if (skip_lfn) goto reset;

            if (fat_is_special_dir(&entry.standard) == 0)
            {
                memcpy(dir_temp_name, entry.standard.short_name, 11);
                dir_temp_name[11] = '\0';
            }

            kprintf("\t%s", dir_temp_name);
            kprintf("\t%s", (const char*[]){"<FILE>", "<DIR>"}[(entry.standard.attributes&ENTRY_ATTR_DIR)>>4]);
            if (!(entry.standard.attributes&ENTRY_ATTR_DIR))
            kprintf("\t%u", entry.standard.file_size);
            kprintf("\n");
            entry_count++;
reset:
            memset(dir_temp_name, 0, 256);
        }
    }
    kprintf("Count: %u item(s)\n", entry_count);
}

int fat_is_special_dir(fat_standard_entry_t* entry)
{
    return (
        (entry->short_name[0] == '.' && entry->short_name[1] == ' ') ||
        (entry->short_name[0] == '.' && entry->short_name[1] == '.' && entry->short_name[2] == ' ')
    ) -1;
}

char dir_temp_name[256];