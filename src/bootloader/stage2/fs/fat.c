#include "fs.h"
#include "partition.h"

int fat_init(fs_t* fs, partition_t* part);

static const fs_ops_t fat_fs_ops = {
    .init = &fat_init,
};

int fat_init(fs_t* fs, partition_t* part)
{
    
}