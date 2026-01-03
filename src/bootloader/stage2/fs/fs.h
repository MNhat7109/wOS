#pragma once
#include "../stdint.h"

#define MODULE_FS "FS"

typedef struct disk_t disk_t;
typedef struct partition_t partition_t;
typedef struct fs_t fs_t;
typedef struct fs_ops_t fs_ops_t;
typedef struct file_t file_t;

typedef struct fs_t
{
    disk_t* disk;
    partition_t* partition;
    fs_ops_t* ops;
    char type[8];
} fs_t;

typedef struct fs_ops_t
{
    int  (*init)(fs_t* fs);
    int  (*open)(fs_t* fs, const char* path, file_t** out);
    u32  (*read)(file_t* file_in, u32 count, void* buffer);
    int  (*traverse)(file_t* file_in);
    void (*close)(file_t* file);
    int (*seek)(file_t* file_in, u32 pos);
} fs_ops_t;

typedef struct file_t
{
    int handle;
    fs_t* fs;
    u8 is_dir;
    usize file_pos;
    usize file_size;
} file_t;

typedef struct file_handle_t
{
    file_t public;
    u8 opened;
    usize first_block;
    usize current_block;
    u32 block_size;
    usize current_pos_in_block; 
} file_handle_t;

typedef const fs_ops_t* (*fs_ops_cb_t)();

int fs_init(const char* fs_type, fs_ops_cb_t cb, fs_t* fs_out);
int fs_mount(fs_t* fs, disk_t* disk, partition_t* part);
void fs_unmount(fs_t* fs);
