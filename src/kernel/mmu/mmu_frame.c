#include <kernel/mmu_frame.h>

void mmu_frame_load_allocator(mmu_frame_allocator_t* m_alloc, const mmu_frame_allocator_t* (*alloc_cb)())
{
    if (!m_alloc) return;
    if (!alloc_cb) return;

    m_alloc = alloc_cb();
}

void (*mmu_frame_init)(u8* start_addr, u64 mem_size) = NULL;

void (*mmu_frame_set_n)(uptr address, usize n) = NULL;
void (*mmu_frame_clear_n)(uptr address, usize n) = NULL;
void (*mmu_frame_reserve_n)(uptr address, usize n) = NULL;

uptr (*mmu_frame_request)(u64 block_size) = NULL;