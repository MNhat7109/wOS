#pragma once
#include "../stdint.h"

#define MBR_MAGIC 0xDECFABAD

typedef struct disk_t disk_t;
int mbr_setup(disk_t* disk);