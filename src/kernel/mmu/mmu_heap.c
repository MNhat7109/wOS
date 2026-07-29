#include <kernel/mmu_heap.h>
#include <kernel/mmu_heap_defs.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu.h>
#include <kernel/debug.h>

struct mmu_heap_shared_data_t mmu_heap_shared_data;

static struct
{
    bool running;
    usize max_size;
    usize footprint;
} mmu_heap_data;

u32 mmu_ceillog2(u64 x);

int mmu_heap_init(void* starting_addr, usize starting_size)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Initializing heap...\n");
    mmu_heap_shared_data.start_range = starting_addr;
    mmu_heap_data.max_size = starting_size;
    mmu_heap_data.running = true;
    kdebugf(DEBUG_INFO, MODULE_MMU, "Initialized heap\n");
}

bool mmu_heap_status()
{
    return mmu_heap_data.running;
}

void mmu_heap_push_chunk_to_list(mmu_heap_node_t** list, mmu_heap_node_t* node)
{
    if (!node) return;

    if (*list == node)
    {
        kdebugf(DEBUG_FATAL, MODULE_MMU, "Bugcheck: To-be-added node == list head\n");
        return;
    }

    node->prev = NULL;
    node->next = (*list);

    if (*list) (*list)->prev = node;
    *list = node;
}

void mmu_heap_remove_chunk_from_list(mmu_heap_node_t** list, mmu_heap_node_t* node)
{
    if (!node) return;

    // Check if node even belongs to the list, or else we can accidentally remove a node from another list
    // NOTE: In order to speed things up, I will not be writing a node traversal code for this. Instead, I can check if certain properties of the node match with the head's.
    // This, unfortunately, can be a problem if either one or more of said properties are modifiable (like a status/owner field), and that such properties are modified BEFORE removing the node from the list.

    if ((*list)->magic != node->magic) return;

    if ((*list)->magic == HEAP_MAGIC_MARK &&
     (*list)->impl.heap.list_type != node->impl.heap.list_type) return;

    if (node->prev) 
    {
        node->prev->next = node->next;
    }
    else 
    {
        *list = node->next;
    }

    if (node->next) 
    {
        node->next->prev = node->prev;
    }
    else 
    {
        *list = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

mmu_heap_node_t* mmu_heap_register_vmm_region(void* start_addr, void* vmm_addr, usize size)
{
    // Align size to 4K before doing absolutely anything else
    size = mmu_align_up(size, PAGE_SIZE);

    // Allocate a page from the VMM for header
    mmu_heap_node_t* hdr = mmu_vmem_alloc(start_addr, PAGE_SIZE, MMU_VMA_ANON | MMU_VMA_R | MMU_VMA_W, NULL);
    if (hdr == ADDR_INVAL)
    {
        // OOM on the VMM side. Therefore, it'll be pointless to continue to the real allocation process
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "VMM said OOM. Can't continue.\n");
        return NULL;
    }

    // Populate header with info:
    // - magic DWORD: VMM_MAGIC_MARK
    // - next, prev: Let's keep this as a linked list, of regions of course
    // - block size = heap size = size
    // - the rest is heap-specific, so reserved

    hdr->magic = VMM_MAGIC_MARK;
    hdr->prev = hdr->next = NULL;
    hdr->block_size = hdr->heap_size = size;
    hdr->impl.vmm_region.address = vmm_addr;

    // Maybe add to another linked list?
    mmu_heap_push_chunk_to_list(&mmu_heap_shared_data.vmm_region_head, hdr);

    // Return the node (more like a header that tells how many bytes the heap requests)
    return hdr;
}

void mmu_heap_unregister_vmm_region(mmu_heap_node_t* node)
{
    if (!node) return;
    if (node->magic != VMM_MAGIC_MARK) return;

    // Need to do 3 things:
    // - Free the actual region
    // - Unlink the node from the linked list
    // - Free the header, or else there will be a black hole

    mmu_heap_remove_chunk_from_list(&mmu_heap_shared_data.vmm_region_head, node);
    mmu_vmem_free(node);
}

mmu_heap_node_t* mmu_heap_spawn_chunk(void* start_addr, usize heap_len, usize block_size)
{
    heap_len = mmu_align_up(heap_len, PAGE_SIZE);
    usize order = mmu_ceillog2((u64)block_size);
    block_size = (usize)(1<<order);
    usize size_class_idx = order - 3;
    bool expanded=false;
    
    if (mmu_heap_data.footprint >= mmu_heap_data.max_size)
    {
        if (mmu_heap_data.max_size+heap_len > MAX_HEAP_SIZE)
        {
            kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Heap limit exceeded\n");
            return NULL;
        }
        mmu_heap_data.max_size+=heap_len;
        expanded = true;
    }
    
    mmu_heap_node_t* node = mmu_heap_shared_data.free_head;
    if (!node)
    {
        // VMM, gimme a range of size (heap_len+page)
        void* chunk = mmu_vmem_alloc(start_addr, heap_len+PAGE_SIZE, MMU_VMA_ANON | MMU_VMA_R | MMU_VMA_W, NULL);
        if (chunk == ADDR_INVAL)
        {
            if (expanded) mmu_heap_data.max_size-=heap_len; // Roll back
            return NULL;
        }
        node = (mmu_heap_node_t*)chunk;
    }

    node->next = NULL;
    node->prev = NULL;
    node->magic = HEAP_MAGIC_MARK;
    node->heap_size = heap_len;
    node->block_size = block_size;
    node->avail_cnt = heap_len/block_size; //
    node->impl.heap.list_type = MMU_HEAP_FREE;
    u8* bmp_buf = (u8*)node+sizeof(mmu_heap_node_t);
    bitmap_init(&node->impl.heap.bmp, bmp_buf, node->avail_cnt);
    node->impl.heap.cur_free_qword = ((u64*)bmp_buf);

    // Add to free list, please. This way, it will ensure that the chunks are updated properly
    if (!mmu_heap_shared_data.free_head)
    {
        mmu_heap_push_chunk_to_list(&mmu_heap_shared_data.free_head, node);
        mmu_heap_register_vmm_region(start_addr, node, 2*PAGE_SIZE);
        mmu_heap_data.footprint += heap_len;
    }


    return node;
}

void mmu_heap_update_chunk_list(mmu_heap_node_t* node)
{
    if (!node) return;

    usize size_class = mmu_ceillog2((u64)node->block_size) - 3;
    
    mmu_heap_node_t** lists[] = {
        &mmu_heap_shared_data.partial_head[size_class], &mmu_heap_shared_data.used_head[size_class], &mmu_heap_shared_data.free_head
    };

    int src_list = node->impl.heap.list_type, dst_list = -1;

    if (node->avail_cnt == 0)
    {
        dst_list = MMU_HEAP_USED;
    }
    if (node->avail_cnt > 0 && node->avail_cnt<node->impl.heap.bmp.bit_count)
    {
        dst_list = MMU_HEAP_PARTIAL;
    }
    if (node->avail_cnt == node->impl.heap.bmp.bit_count)
    {
        dst_list = MMU_HEAP_FREE;
    }

    if (src_list == dst_list) return;

    {
        const char* disp[] = {"PARTIAL", "USED", "FREE"};
        kdebugf(DEBUG_INFO, MODULE_MMU, "Changing status of chunk 0x%p: %s -> %s\n", node, disp[src_list], disp[dst_list]);
    }

    mmu_heap_remove_chunk_from_list(lists[src_list], node);
    mmu_heap_push_chunk_to_list(lists[dst_list], node);

        kdebugf(DEBUG_INFO, MODULE_MMU, "current list=0x%p -> new list=0x%p\n", *lists[src_list], *lists[dst_list]);

    // NOTE: Don't put this code before removing the node from the list. Because there is a HACK in the node removal function itself, doing so will nullify the HACK and therefore, create bugs.
    // (see mmu_heap_remove_chunk_from_list())

    node->impl.heap.list_type = dst_list;
}
