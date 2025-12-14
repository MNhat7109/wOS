#pragma once
#include "../stdint.h"

typedef enum 
{ 
    MEDIA_TYPE_CDROM,
    MEDIA_TYPE_HD,
    MEDIA_TYPE_FD,
    MEDIA_TYPE_USB,
} disk_media_type_t;

typedef enum
{
    CTL_TYPE_IDE,
} disk_ctl_type_t;

typedef struct disk_t
{
    int ctl_type;
    void* ctrl;
    int media_type;
    u32 drive_number;
    u8 specific[32];
} disk_t;

void disk_init();