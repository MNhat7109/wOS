#pragma once
#include "../stdint.h"
#include <stdbool.h>

typedef struct
{
    int file_handle_no;
    bool is_directory;
    u32 file_pos;
    u32 file_size;
} FAT_file_t;

typedef struct
{
    char short_name[11];
    u8 attributes;
    u8 _nt_reserved;
    u8 creation_time_tenth;
    u16 creation_time;
    u16 creation_date;
    u16 last_accessed_date;
    u16 first_cluster_hi;
    u16 last_mod_time;
    u16 last_mod_date;
    u16 first_cluster_lo;
    u32 file_size;
} __attribute__((packed)) FAT_dir_entry;

typedef struct
{
    u8 order;
    u16 first_chunk[5];
    u8 attribute;
    u8 entry_type;
    u8 checksum;
    u16 next_chunk[6];
    u16 zero;
    u16 final_chunk[2];
} __attribute__((packed)) FAT_lfn_t;

bool FAT_init(u8 ata_drive_number, void* partition_offset);
FAT_file_t* FAT_open(const char* path);
u32 FAT_read(FAT_file_t* file, u32 count, void* buffer);
bool FAT_read_entry(FAT_file_t* file, void* entry);
void FAT_close(FAT_file_t* file);

void FAT_seek(FAT_file_t* file, u32 pos);