#include <kernel/mmu_frame.h>

void mmu_frame_load_allocator(mmu_frame_allocator_t* m_alloc, const mmu_frame_allocator_t* (*alloc_cb)())
{
    if (!m_alloc) return;
    if (!alloc_cb) return;

    m_alloc = alloc_cb();
}

