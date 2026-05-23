#include <kernel/mmu_frame.h>

static mmu_frame_allocator_t this_alloc;

uptr mmu_frame_get_meta_offset()
{
    return this_alloc.meta_offset_vaddr;
}

u64 mmu_frame_get_meta_size()
{
    return this_alloc.meta_size;
}

int mmu_frame_populate(mmu_frame_plugins_t* plugin, uptr offset, u64 size)
{
    if (!plugin) return -1;
    if (!plugin->init) return -1;

    int status = plugin->init(&this_alloc, offset, size);
    if (status < 0) return status;

    this_alloc.plugin = plugin;
    return 0;
}

uptr mmu_frame_alloc(u64 n)
{
    if (!this_alloc.plugin) 
    {
        return 0;
    }

    if (!this_alloc.plugin->ops.alloc) return 0;
    return this_alloc.plugin->ops.alloc(&this_alloc, n);
}

void mmu_frame_free(uptr paddr)
{
    if (!this_alloc.plugin) 
    {
        return;
    }

    if (!this_alloc.plugin->ops.free) return;
    this_alloc.plugin->ops.free(&this_alloc, paddr);
}

void mmu_frame_reserve_pages(uptr paddr, u64 n)
{
    if (!this_alloc.plugin) 
    {
        return;
    }

    if (!this_alloc.plugin->ops.reserve_pages) return;
    this_alloc.plugin->ops.reserve_pages(&this_alloc, paddr, n);  
}

void mmu_frame_lock_pages(uptr paddr, u64 n)
{
    if (!this_alloc.plugin) 
    {
        return;
    }

    if (!this_alloc.plugin->ops.lock_pages) return;
    this_alloc.plugin->ops.lock_pages(&this_alloc, paddr, n);  
}
void mmu_frame_release_pages(uptr paddr, u64 n)
{
    if (!this_alloc.plugin) 
    {
        return;
    }

    if (!this_alloc.plugin->ops.release_pages) return;
    this_alloc.plugin->ops.release_pages(&this_alloc, paddr, n);  
}