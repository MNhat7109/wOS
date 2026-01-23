#pragma once
#include "stdint.h"

typedef struct framebuffer_t
{
    u32 base;
    u32 size;
    u32 height, width;
    u32 bpp, pitch;
    u32 xpos, ypos;
} __attribute__((packed)) framebuffer_t;

typedef struct font_t
{
    u8 height, width;
    u16 glyph_count;
    void* glyph; 
} __attribute__((packed)) font_t;

typedef struct memory_info_t
{
    u32 entries_count;
    struct
    {
        u64 base, length;
        u32 type;
        u32 acpi;
    } __attribute__((packed)) regions[160];
    u8 _padding[252];
} __attribute__((packed)) memory_info_t;

typedef struct system_desc_ptr_t system_desc_ptr_t;

typedef struct boot_info_t
{
    framebuffer_t* framebuffer;
    system_desc_ptr_t* sdp;
    font_t* font_out;
    memory_info_t* mem_map;
} boot_info_t;
