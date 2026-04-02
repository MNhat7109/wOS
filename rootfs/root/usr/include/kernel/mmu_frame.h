#pragma once
#include <stdint.h>
#include <bitmap.h>

typedef enum 
{
    MMU_FRAME_ALLOC_FLAG_NORMAL,
    MMU_FRAME_ALLOC_FLAG_RESERVED,
} mmu_frame_alloc_flags_t;

typedef struct mmu_frame_allocator_t
{
    bitmap_t* mem_state;
    void (*init)(mmu_frame_allocator_t* f_alloc, u8* offset, u64 limit);
    uptr (*alloc)(mmu_frame_allocator_t* f_alloc, u64 block_size, int flags);
    void (*free)(mmu_frame_allocator_t* f_alloc, uptr phys_addr);
} mmu_frame_allocator_t;

void mmu_frame_load_allocator(mmu_frame_allocator_t* m_alloc, const mmu_frame_allocator_t* (*alloc_cb)());

extern void (*mmu_frame_init)(u8* start_addr, u64 mem_size);

extern void (*mmu_frame_set_n)(uptr address, usize n);
extern void (*mmu_frame_clear_n)(uptr address, usize n);
extern void (*mmu_frame_reserve_n)(uptr address, usize n);

extern uptr (*mmu_frame_request)(u64 block_size);