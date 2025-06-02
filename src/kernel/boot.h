#pragma once
#include "video/video.h"
#include "memory/memory.h"

extern u8 __bss_start;
extern u8 __start;
extern u8 __end;

typedef struct 
{
    void* partition_offset;
    framebuffer_t* framebuffer;
    font_t* font_out;
    memory_info_t* mem_map;
} boot_info_t;