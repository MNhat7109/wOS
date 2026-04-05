#pragma once
#include <kernel/mmu_frame.h>

typedef struct mmu_frame_bmp_allocator_ops_t
{
    mmu_frame_allocator_ops_t hdr;
    void (*lock_pages)(uptr address, usize n);
    void (*reserve_pages)(uptr address, usize n);
    void (*free_pages)(uptr address, usize n);
} mmu_frame_bmp_allocator_ops_t;

const mmu_frame_allocator_ops_t* mmu_frame_bmp_load_ops();
