#pragma once
#include "../stdint.h"

typedef struct partition_t partition_t;
typedef struct fs_t fs_t;
typedef struct fs_ops_t fs_ops_t;
typedef struct file_t file_t;

typedef struct fs_t
{
    partition_t* partition;
    fs_ops_t* ops;
    char type[8];
} fs_t;

typedef struct fs_ops_t
{
    int  (*init)(fs_t* fs, partition_t* partition);
    int  (*open)(fs_t* fs, const char* path, file_t** out);
    u32  (*read)(file_t* file_in, u32 count, void* buffer);
    int  (*traverse)(file_t* file_in);
    void (*close)(file_t* file);
    void (*seek)(file_t* file_in, u32 pos);
} fs_ops_t;

typedef struct file_t
{
    int handle;
    fs_t* fs;
    u8 is_dir;
    u32 file_pos;
    u32 file_size;
} file_t;

int fs_init(const char* fs_type, fs_t* fs_out);
int fs_mount(fs_t* fs, partition_t* part);
void fs_unmount(fs_t* fs);
