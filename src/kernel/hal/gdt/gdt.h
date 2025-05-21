#pragma once
#include "../../stdint.h"
#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10
#define USER_CODE_SEG   0x18
#define USER_DATA_SEG   0x20

void GDT_init();