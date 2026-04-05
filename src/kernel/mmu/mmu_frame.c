#include <kernel/mmu_frame.h>

mmu_frame_allocator_t default_alloc;

void mmu_frame_load_allocator(mmu_frame_allocator_t** m_alloc)
{
    *m_alloc = &default_alloc;
}

void mmu_frame_load_ops(mmu_frame_allocator_t* m_alloc, const mmu_frame_allocator_ops_t* (*alloc_cb)())
{
    if (!alloc_cb) return;

    (m_alloc)->ops = alloc_cb();
}
