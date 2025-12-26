#include "fs.h"
#include "../string/string.h"

int fs_init(const char* fs_type, fs_t* fs_out)
{
    memcpy(fs_out->type, fs_type, sizeof(fs_out->type));
    return 0;
}

int fs_mount(fs_t* fs, partition_t* part)
{
    fs->partition = part; 
    int status = fs->ops->init(fs, part);
    return status;
}

void fs_unmount(fs_t* fs)
{
    fs->partition = NULL;
}