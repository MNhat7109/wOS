#pragma once
#include <stdint.h>
#include <bitmap.h>

typedef struct mmu_frame_allocator_t mmu_frame_allocator_t;

typedef struct mmu_frame_allocator_t
{
    bitmap_t* mem_state;
    void (*init)(mmu_frame_allocator_t* f_alloc, u8* offset, u64 limit);
    uptr (*alloc)(mmu_frame_allocator_t* f_alloc, u64 block_size);
    void (*free)(mmu_frame_allocator_t* f_alloc, uptr phys_addr);
} mmu_frame_allocator_t;

void mmu_frame_load_allocator(mmu_frame_allocator_t* m_alloc, const mmu_frame_allocator_t* (*alloc_cb)());

