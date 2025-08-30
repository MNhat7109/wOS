#pragma once

typedef struct boot_info_t
{
    void* partition_offset;
    void* framebuffer;
    void* sdp;
    void* font_out;
    void* mem_map;
} boot_info_t;