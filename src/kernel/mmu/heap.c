#include <kutils/mmu/heap.h>
#include <libk/mmu/page_allocator.h>
#include <libk/mmu/mmu.h>
#include <libk/bitmanip/bitmanip.h>

#include <kutils/mmu/buddy_allocator.h>
#include <kutils/mmu/slab_allocator.h>

#include <libk/stdio.h>
#include <libk/string.h>

typedef enum heap_block_owner_t
{
    BLOCK_OWNER_SLAB,
    BLOCK_OWNER_BUDDY,
    BLOCK_OWNER_UNKNOWN
} heap_block_owner_t;

#define ALLOC_MAGIC_NUMBER 0xA110CCED
#define FREE_MAGIC_NUMBER 0xDEADBABE

typedef struct heap_block_attribute_t
{
    usize magic;
    usize size;
    usize owner;
    usize padding;
} heap_block_attribute_t;

static struct heap_t
{
    usize heap_page_count;
    usize slab_page_count;
    usize buddy_page_count;
    void* heap_base;
    void* slab_alloc_base;
    void* buddy_alloc_base;
} heap_data;

#define MAX_SLAB_PAGE_CNT(_pc) ((_pc)/2)
#define MAX_BUDDY_PAGE_CNT(_pc) ((_pc)/2)

static heap_block_attribute_t* mmu_heap_get_hdr(void* ptr);

void mmu_init_heap(void* address, usize page_count)
{
    // Map the whole memory chunk that the heap will use
    // We will start by requesting physical memory, then we'll map it to
    // our virtual address in the higher half (optimally from 0xC0000000 onward)
    
    heap_data.heap_base = address;

    u8* heap_vaddr = (u8*)address;
    for (usize i=0;i<page_count;i++)
    {
        void* heap_paddr = page_alloc_request();
        mmu_mmap((usize)(heap_vaddr+i*0x1000), (usize)heap_paddr, MMU_PT_FLAG_READ_WRITE);
    }

    // Populate the heap data
    heap_data.heap_page_count = page_count;

    // Optimally, we will split the heap to two regions: the Slab and the Buddy
    // Each of them will have its own thresholds
    // To be optimal, we will split the heap in half, so that
    // both the slab and the buddy allocator can inherit the same amount of memory
    // We can change this in the macros, though, by changing the divisors.

    heap_data.slab_page_count = MAX_SLAB_PAGE_CNT(page_count);
    heap_data.buddy_page_count = MAX_BUDDY_PAGE_CNT(page_count);

    heap_data.slab_alloc_base = heap_vaddr;
    heap_data.buddy_alloc_base = heap_vaddr+heap_data.slab_page_count*0x1000;

    // Finally, pass everything we got to each of the respective allocators.
    slab_alloc_init(heap_data.slab_alloc_base, heap_data.slab_page_count);
    buddy_alloc_init(heap_data.buddy_alloc_base, heap_data.buddy_page_count);

    kprintf("Heap: Heap is now activated.\n");
}

void mmu_destroy_heap()
{
    // Call destructor in each allocator, so that they can have time
    // to deallocate any existing memory in their containers
    slab_alloc_destroy();
    buddy_alloc_destroy();

    // Now we deallocate the heap itself
    usize free_count = heap_data.heap_page_count;
    for (usize i=0;i<free_count;i++)
    {
        usize paddr = mmu_munmap((usize)(heap_data.heap_base+i*0x1000));
        if (!paddr)
        {
            // In some weird cases of unmapping non-existent pages, 
            // we can just holler back, then return.
            // Nobody will have to be hurt after this.

            kprintf("Heap: Stumbled beyond the threshold. Returning...\n");
            return;
        }

        // Otherwise, we got our needed physical address. just free it.
        page_alloc_free(paddr);
    }

    kprintf("Heap: Heap is now deactivated.\n");
}

void* mmu_allocate(usize size)
{
    // In the previous version of the MMU, the buddy is used as a
    // "fallback" if the slab allocation is unsuccessful

    // That is actually dumb, because doing this, we intentionally
    // fragment the buddy list further, which means there will be more
    // entries added to the buddy linked list 
    // (or in our case, a doubly linked binary tree), therefore it will waste
    // heaploads of memory for those newly added entries, and eventually,
    // we cannot add larger chunks of memory anymore (since it's fragmented)

    // So, for this version of MMU, if the slab fails, just return NULL.
    // No fallback to buddy allocator, no nothing.
    // Same can be said for the buddy allocator.
    // It will be used ONLY if our to-be-allocated pointer size 
    // exceeds the maximum pointer size allowed in the slab allocator.

    void* ptr = NULL;
    bool slab, buddy;
    heap_block_attribute_t* hdr;

    if (size <= slab_alloc_get_max_ptr_size())
    {
        kprintf("Heap: Size of the pointer is %u bytes. Slab allocator will be used\n", size);
        ptr = slab_alloc_request(size+sizeof(heap_block_attribute_t));
        if (!ptr) return NULL;

        hdr = (heap_block_attribute_t*)ptr;
        hdr->owner = BLOCK_OWNER_SLAB;
    }
    else
    {
        kprintf("Heap: Size of the pointer is %u bytes. Buddy allocator will be used instead\n", size);
        ptr = buddy_alloc_request(size+sizeof(heap_block_attribute_t));
        if (!ptr) return NULL;

        hdr = (heap_block_attribute_t*)ptr;
        hdr->owner = BLOCK_OWNER_BUDDY;
    }

    hdr->size = size;
    hdr->magic = ALLOC_MAGIC_NUMBER;
    
    return (void*)(hdr+1); // Only return the fertile part
}

void* mmu_reallocate(void* ptr, usize size)
{
    if (!ptr) return mmu_allocate(size);
    if (size == 0)
    {
        mmu_free(ptr);
        return NULL;
    }

    // Access the header of the old location
    heap_block_attribute_t* hdr = mmu_heap_get_hdr(ptr);

    if (hdr->size >= size) return ptr;

    void* new_ptr = mmu_allocate(size);
    if (!new_ptr) return NULL;

    usize to_copy = hdr->size;
    memcpy(new_ptr, ptr, to_copy);
    
    // Free the old loaction
    mmu_free(ptr);
    return new_ptr;
}

void mmu_free(void* block)
{
    // Access the header
    heap_block_attribute_t* hdr = mmu_heap_get_hdr(block);

    if (hdr->magic != ALLOC_MAGIC_NUMBER)
    {
        if (hdr->magic == FREE_MAGIC_NUMBER)
            kprintf("Heap: Double free detected\n");
        else kprintf("Heap: Invalid block detected\n");
        return;
    }
    
    hdr->magic = FREE_MAGIC_NUMBER;

    // We always want to free the WHOLE THING, including the header.
    // Not doing so may result in memory leftovers, which if left unpaid attention on,
    // will accumulate over time, causing massive memory leaks.

    switch (hdr->owner)
    {
        case BLOCK_OWNER_SLAB:
            slab_alloc_free(hdr);
            break;
        case BLOCK_OWNER_BUDDY:
            buddy_alloc_free(hdr);
            break;
        default:
            kprintf("Heap: This block of memory is a 'stray' block."
                " That means MMU cannot find an owner of the memory"
                " to request the memory reclaim.\n");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static heap_block_attribute_t* mmu_heap_get_hdr(void* ptr)
{
    return (heap_block_attribute_t*)((u8*)ptr-sizeof(heap_block_attribute_t));
}