#include <kernel/mmu_vmem.h>
#include <kernel/mmu_heap.h>
#include <kernel/mmu_heap_defs.h>
#include <kernel/debug.h>

u32 mmu_ceillog2(u64 x);
mmu_heap_node_t* mmu_heap_register_vmm_region(void* start_addr, void* vmm_addr, usize size);
void mmu_heap_update_chunk_list(mmu_heap_node_t* node);
mmu_heap_node_t* mmu_heap_spawn_chunk(void* start_addr, usize heap_len, usize block_size);

void mmu_heap_get_next_free_qword(mmu_heap_node_t* node)
{
    usize qword_cnt = mmu_align_up(node->impl.heap.bmp.bit_count, 64) / 64;
    usize cur_idx = (node->impl.heap.cur_free_qword-(u64*)node->impl.heap.bmp.buffer);
    for (usize i=0; i<qword_cnt;i++)
    {
        u64* qword_ptr = ((u64*)node->impl.heap.bmp.buffer)+((cur_idx+i)%qword_cnt);

        if (*qword_ptr != (u64)~0)
        {
            node->impl.heap.cur_free_qword = qword_ptr;
            return;
        }
    }

    node->impl.heap.cur_free_qword = NULL;
}

void* mmu_heap_alloc_vmem(usize size)
{
    mmu_heap_node_t* node;
    void* addr = mmu_vmem_alloc(mmu_heap_shared_data.start_range, size, MMU_VMA_ANON | MMU_VMA_R | MMU_VMA_W, NULL);
    if (addr == ADDR_INVAL)
    {
        // OOM!
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "VMM said OOM\n");
        return NULL;
    }

    // Register a "region" so that the VMM passthrough is kept track of.
    node = mmu_heap_register_vmm_region(mmu_heap_shared_data.start_range, addr, size);

        kdebugf(DEBUG_INFO, MODULE_MMU, "Allocated %u bytes (VMM-delegated) at addr=0x%p\n", 
        size, 
        node->impl.vmm_region.address
    );
    return node->impl.vmm_region.address;
}

void* mmu_heap_alloc_regular(usize size)
{
    mmu_heap_node_t* node;

    usize order = mmu_ceillog2((u64)size);
    usize size_class_idx = order - 3;

    // Obtain chunks eligible for allocation
    node = mmu_heap_shared_data.partial_head[size_class_idx];
    if (node) 
    {
        goto next_step;
    }

    kdebugf(DEBUG_INFO, MODULE_MMU, "Looking for free chunks...\n");
    node = mmu_heap_spawn_chunk(mmu_heap_shared_data.start_range, PAGE_SIZE, size);
    if (!node) 
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Ran out of free chunks.\n");
        return NULL;
    }
next_step:

    // Mark bit as footprint in bitmap
    if (node->avail_cnt == 0)
    {
        // Bugcheck, cause why would a full, footprint chunk be either in the free or partial list?
        kdebugf(DEBUG_FATAL, MODULE_MMU, "Bugcheck: Forgot to put fully used chunk in the full list\n");
        return NULL;
    }

    if (!node->impl.heap.cur_free_qword)
    {
        kdebugf(DEBUG_FATAL, MODULE_MMU, "Bugcheck: cur_free_qword=0x0 in the free/partial list\n");
        return NULL;
    }

    usize idx = (node->impl.heap.cur_free_qword-(u64*)node->impl.heap.bmp.buffer)*64 + 
    __builtin_ctzll(~*node->impl.heap.cur_free_qword);

    bitmap_set(&node->impl.heap.bmp, idx);
    node->avail_cnt--;

    // Assign next free bit
    if (*node->impl.heap.cur_free_qword == (u64)~0)
    {
        mmu_heap_get_next_free_qword(node);
    }

    // Update chunk after marking. 
    mmu_heap_update_chunk_list(node);

        kdebugf(DEBUG_INFO, MODULE_MMU, "Allocated %u bytes to %u-byte chunk at addr=0x%p\n", 
        size, 
        node->block_size,
        (u8*)node+PAGE_SIZE+idx*node->block_size
    );
    return (u8*)node+PAGE_SIZE+idx*node->block_size;
}

void* mmu_heap_alloc(usize size)
{
    if (!size) return NULL;

    kdebugf(DEBUG_INFO, MODULE_MMU, "Allocating %u bytes from heap...\n", size);

    // If the requested size is too big, gotta ask the VMM directly for free memory
    // Also have to let the heap know that the chunk is from the VMM.
    if (size > (1<<(MAX_HEAP_ORDER+3)))
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "%u bytes is too big (>2048 bytes), delegating to VMM...\n", size);
        return mmu_heap_alloc_vmem(size);
    }

    // Round up if the requested size is less than the minimum block size
    if (size < (1<<(MIN_HEAP_ORDER+3))) size = (1<<(MIN_HEAP_ORDER+3));
    return mmu_heap_alloc_regular(size);
}