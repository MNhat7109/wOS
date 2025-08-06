#pragma once
#include <stdbool.h>
#include "stdint.h"

extern u8 __bss_start;
extern u8 __start;
extern u8 __end;

typedef struct 
{
    void* partition_offset;
    void* framebuffer;
    void* sdp;
    void* font_out;
    void* mem_map;
} boot_info_t;

extern boot_info_t* bootloader_info;
bool boot_prepare(boot_info_t* info);
void boot_prepare_acpi();
