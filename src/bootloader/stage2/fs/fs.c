#include "fs.h"

#include "../stdio.h"
#include "../string/string.h"

int fs_init(const char* fs_type, fs_ops_cb_t cb, fs_t* fs_out)
{
    u8 size = strlen(fs_type)<sizeof((fs_out)->type)?
    strlen(fs_type):
    sizeof((fs_out)->type);
    
    memcpy((fs_out)->type, fs_type, size);
    if (!cb) return -1;
    (fs_out)->ops = cb();
    return 0;
}

int fs_mount(fs_t* fs, disk_t* disk, partition_t* part)
{
    fs->partition = part; fs->disk = disk;
    int status = 0;
    status = fs->ops->init(fs);

    if (status < 0)
    kdebugf(DEBUG_CRITICAL, MODULE_FS, "FS mounting failed\n");

    return status;
}

void fs_unmount(fs_t* fs)
{
    fs->disk = NULL;
    fs->partition = NULL;
}