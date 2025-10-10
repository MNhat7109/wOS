#pragma once

#include <libk/stdint.h>

typedef enum
{
    MMU_FRAME_TYPE_FREE = 0,
    MMU_FRAME_TYPE_USED = 1,
    MMU_FRAME_TYPE_RESERVED = 2,
    MMU_FRAME_TYPE_HOLE = 3,
    MMU_FRAME_TYPE_BAD = 4,
    MMU_FRAME_TYPE_OTHER = 0xFF,
} mmu_frame_region_type_t;

void mmu_frame_init(void* addr);

void mmu_frame_set(usize size, int type);

u64 mmu_frame_create(usize size);
void mmu_frame_free(u64 addr);

usize mmu_frame_get_size(int type);