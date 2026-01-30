#pragma once
#include <stdint.h>

typedef struct framebuffer_t framebuffer_t;
typedef struct system_desc_ptr_t system_desc_ptr_t;
typedef struct font_t font_t;
typedef struct  memory_info_t memory_info_t;

typedef struct boot_info_t
{
    framebuffer_t* framebuffer;
    system_desc_ptr_t* sdp;
    font_t* font_out;
    memory_info_t* mem_map;
    u32 optional_params;
} boot_info_t;

typedef enum
{
    PARAM_PAE_ON = (1<<0),
    PARAM_NX_ON = (1<<1),
    PARAM_PSE_ON = (1<<2),
} optional_param_t;
