#include <kernel/mmu_heap.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu.h>
#include <kernel/kernel_defs.h>
#include <bitmap.h>

#define MAX_SIZE_CLASS_CNT 9
#define MIN_HEAP_ORDER 0
#define MAX_HEAP_ORDER 8
#define MAGIC_MARK 0xBADD1E6F

typedef struct mmu_heap_node_t mmu_heap_node_t;

typedef enum
{
    MMU_HEAP_PARTIAL,
    MMU_HEAP_USED,
    MMU_HEAP_FREE,
} mmu_heap_status_t;

typedef struct mmu_heap_node_t
{
    u32 magic;
    mmu_heap_node_t* prev;
    mmu_heap_node_t* next;
    u32 block_size;
    u32 heap_size;
    u32 avail_cnt;
    int list_type;
    u64* cur_free_qword;
    bitmap_t bmp;
} mmu_heap_node_t;

typedef mmu_heap_node_t* mmu_heap_list_t[MAX_SIZE_CLASS_CNT];

static struct
{
    void* start_range;
    usize max_size;
    usize used;
    mmu_heap_list_t free_head;
    mmu_heap_list_t used_head;
    mmu_heap_list_t partial_head;
} mmu_heap_data;

u32 mmu_ceillog2(u64 x);

int mmu_heap_init(void* starting_addr, usize starting_size)
{
    mmu_heap_data.start_range = starting_addr;
    mmu_heap_data.max_size = starting_size;
}

void mmu_heap_add_chunk_to_list(mmu_heap_list_t list, usize size_class, mmu_heap_node_t* node)
{
    if (!node) return;
    if (size_class >= MAX_SIZE_CLASS_CNT) return;

    if (list[size_class]) list[size_class]->prev = node;
    node->next = list[size_class];
    list[size_class] = node;
}

void mmu_heap_remove_chunk_from_list(mmu_heap_list_t list, usize size_class, mmu_heap_node_t* node)
{
    if (!node) return;
    if (size_class >= MAX_SIZE_CLASS_CNT) return;

    if (list[size_class] == node) list[size_class] = node->next;

    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
}

void mmu_heap_pop_chunk_from_list(mmu_heap_list_t list, usize size_class)
{
    if (size_class >= MAX_SIZE_CLASS_CNT) return;

    mmu_heap_node_t* node = list[size_class];
    mmu_heap_remove_chunk_from_list(list, size_class, node);
}

mmu_heap_node_t* mmu_heap_spawn_chunk(void* start_addr, usize heap_len, usize block_size)
{
    if (mmu_heap_data.used >= mmu_heap_data.max_size)
    {
        // TODO
    }
    
    usize order = mmu_ceillog2((u64)block_size);
    block_size = (usize)(1<<order);
    usize size_class_idx = order - 3;
    
    mmu_heap_node_t* node = mmu_heap_data.free_head[0];
    if (!node)
    {
        // VMM, gimme a range of size (heap_len+page)
        heap_len = mmu_align_up(heap_len, PAGE_SIZE);
        void* chunk = mmu_vmem_alloc(start_addr, heap_len+PAGE_SIZE, MMU_VMA_ANON | MMU_VMA_R | MMU_VMA_W, NULL);
        if (!chunk)
        {
            return NULL;
        }
        node = (mmu_heap_node_t*)chunk;
    }

    node->next = NULL;
    node->prev = NULL;
    node->magic = MAGIC_MARK;
    node->heap_size = heap_len;
    node->block_size = block_size;
    node->avail_cnt = heap_len/block_size; //
    node->list_type = MMU_HEAP_FREE;
    u8* bmp_buf = (u8*)node+sizeof(mmu_heap_node_t);
    bitmap_init(&node->bmp, bmp_buf, node->avail_cnt);
    node->cur_free_qword = &((u64*)bmp_buf)[0];

    mmu_heap_data.used += heap_len+PAGE_SIZE;

    return node;
}

void mmu_heap_update_chunk_list(mmu_heap_node_t* node)
{
    if (!node) return;

    usize size_class = mmu_ceillog2((u64)node->block_size) - 3;

    mmu_heap_list_t lists[] = {mmu_heap_data.partial_head, mmu_heap_data.used_head, mmu_heap_data.free_head};
    usize sizes[] = { size_class, size_class, 0};

    int src_list = node->list_type, dst_list = -1;

    if (node->avail_cnt == 0)
    {
        dst_list = MMU_HEAP_USED;
    }
    if (node->avail_cnt > 0 && node->avail_cnt<node->bmp.bit_count)
    {
        dst_list = MMU_HEAP_PARTIAL;
    }
    if (node->avail_cnt == node->bmp.bit_count)
    {
        dst_list = MMU_HEAP_FREE;
    }

    if (src_list == dst_list) return;

    mmu_heap_remove_chunk_from_list(lists[src_list], sizes[src_list], node);
    mmu_heap_add_chunk_to_list(lists[dst_list], sizes[dst_list], node);
}

void mmu_heap_get_next_free_qword(mmu_heap_node_t* node)
{
    usize qword_cnt = mmu_align_up(node->bmp.bit_count, 64) / 64;
    usize cur_idx = (node->cur_free_qword-(u64*)node->bmp.buffer);
    for (usize i=0; i<qword_cnt;i++)
    {
        u64* qword_ptr = ((u64*)node->bmp.buffer)+((cur_idx+i)%qword_cnt);

        if (*qword_ptr != (u64)~0)
        {
            node->cur_free_qword = qword_ptr;
            return;
        }
    }

    node->cur_free_qword = NULL;
}

mmu_heap_node_t* mmu_heap_find_header(void* ptr)
{
    
}

void* mmu_heap_alloc(usize size)
{
    if (!size) return NULL;

    // If the requested size is too big, gotta ask the VMM directly for free memory
    // Also have to let the heap know that the chunk is from the VMM.
    if (size >= (1<<(MAX_HEAP_ORDER+3)))
    {
        // Register a "region" so that the VMM passthrough is kept track of.
    }

    // Round up if the requested size is less than the minimum block size
    if (size < (1<<(MIN_HEAP_ORDER+3))) size = (1<<(MIN_HEAP_ORDER+3));

    // Obtain chunks eligible for allocation
    usize order = mmu_ceillog2((u64)size);
    usize size_class_idx = order - 3;

    mmu_heap_node_t* node = mmu_heap_data.partial_head[size_class_idx];
    if (node) goto next_step;
    node = mmu_heap_spawn_chunk(mmu_heap_data.start_range, PAGE_SIZE, size);
    if (!node) return NULL;
next_step:
    // Mark bit as used in bitmap
    if (node->avail_cnt == 0)
    {
        // Bugcheck, cause why would a full chunk be either in the free or partial list?
        return NULL;
    }
    usize idx = __builtin_ctzll(~*node->cur_free_qword);
    bitmap_set(&node->bmp, idx);
    node->avail_cnt--;

    // Assign next free bit
    if (*node->cur_free_qword == (u64)~0)
    {
        mmu_heap_get_next_free_qword(node);
    }

    // Update chunk after marking. 
    mmu_heap_update_chunk_list(node);

    // Return the pointer
    return (u8*)node+PAGE_SIZE+idx*node->block_size;
}

void mmu_heap_free(void* ptr)
{
    // Find the header in which the pointer resides

    mmu_heap_node_t* node = mmu_heap_find_header(ptr);
    if (!node)
    {
        // Heap does not own this chunk, or at least acknowledge it as a VMM chunk
        return;
    }

    // Find the bit position the pointer resides
    u8* user_data = (u8*)node+PAGE_SIZE;
    usize bit_pos = ((u8*)ptr-user_data)/node->block_size;

    // Clear the bit if set, freak out otherwise.
    if (bitmap_get(&node->bmp, bit_pos) == 0)
    {
        // error!
        return;
    }

    bitmap_clear(&node->bmp, bit_pos);
    node->avail_cnt++;

    // Update the chunk status (Set status, move chunk to another list etc.)
    mmu_heap_update_chunk_list(node);
}