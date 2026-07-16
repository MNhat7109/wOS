#pragma once
#include <stdint.h>

typedef struct mmu_frame_buddy_t mmu_frame_buddy_t;

typedef struct mmu_frame_buddy_attr_t
{
    u8 reserved : 1;
    u8 is_free : 1;
    u8 zone : 2;
    u8 order: 4;
    u8 is_head : 1;
    u8 padding : 3;
} __attribute__((packed)) mmu_frame_buddy_attr_t;

typedef struct mmu_frame_buddy_t
{
    mmu_frame_buddy_attr_t frame_attr;
    mmu_frame_buddy_t* next;
} mmu_frame_buddy_t;
