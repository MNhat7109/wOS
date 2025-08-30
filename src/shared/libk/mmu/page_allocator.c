#include <libk/mmu/mmu.h>
#include <libk/stdint.h>
#include <libk/mmu/page_allocator.h>
#include <libk/bitmap/bitmap.h>

static struct
{
    volatile usize free_mem;
    volatile usize used_mem;
    volatile usize reserved_mem;
    volatile usize last_free_idx;
    bitmap_t page_bitmap;
} page_alloc_data;

void page_alloc_init(void* bitmap_base)
{
    usize total_ram_bytes = mmu_get_memory_size();
    usize total_page_count_bits = mmu_byte_to_page_count(total_ram_bytes);

    page_alloc_data.free_mem = total_ram_bytes;
    page_alloc_data.used_mem = 0;
    page_alloc_data.reserved_mem = 0;

    bitmap_init(&page_alloc_data.page_bitmap, total_page_count_bits, bitmap_base);

    usize bmp_page_count = mmu_byte_to_page_count(page_alloc_data.page_bitmap.size);
    page_alloc_lockn(page_alloc_data.page_bitmap.buffer, bmp_page_count);

    memory_info_t* mem_map = mmu_get_memory_map();
    usize ent_cnt = mem_map->entries_count;
    for (usize i=0;i<ent_cnt;i++)
    {
        if (mem_map->regions[i].type != MEMORY_TYPE_FREE)
        {
            usize num_pages = mmu_byte_to_page_count(mem_map->regions[i].length);
            page_alloc_reserven((void*)mem_map->regions[i].base, num_pages);
        }
    }
}

void page_alloc_free(void* address)
{
    // Spinlock here
    usize idx = (usize)address >> 12;

    // Spin-unlock here
    if (!bitmap_get_bits(&page_alloc_data.page_bitmap,idx)) return;
    bitmap_clear_bits(&page_alloc_data.page_bitmap, idx);

    page_alloc_data.free_mem += 4096;
    page_alloc_data.used_mem -= 4096;
    // Spin-unlock here
}

void page_alloc_lock(void* address)
{
    // Spinlock here
    usize idx = (usize)address >> 12;

    // Spin-unlock here
    if (bitmap_get_bits(&page_alloc_data.page_bitmap,idx)) return;
    bitmap_set_bits(&page_alloc_data.page_bitmap, idx);

    page_alloc_data.free_mem -= 4096;
    page_alloc_data.used_mem += 4096;
    // Spin-unlock here
}

void page_alloc_freen(void* address, usize page_count)
{
    for (usize i=0;i<page_count;i++)
    {
        // Spinlock here
        page_alloc_free(address+(i<<12));
        // Spin-unlock here
    }
}

void page_alloc_lockn(void* address, usize page_count)
{
    for (usize i=0;i<page_count;i++)
    {
        // Spinlock here
        page_alloc_lock(address+(i<<12));
        // Spin-unlock here
    }
}

// Find the next free page using the bitmap.
// Starts from last known free index to improve performance.

void* page_alloc_request()
{
    // Spinlock here

    usize bitmap_bit_cnt = page_alloc_data.page_bitmap.size<<3;
    if (page_alloc_data.last_free_idx >= bitmap_bit_cnt)
        page_alloc_data.last_free_idx = 0; // wrap around
    
    for (usize i=page_alloc_data.last_free_idx;i<bitmap_bit_cnt;i++)
    {
        if (bitmap_get_bits(&page_alloc_data.page_bitmap, i)) continue;

        page_alloc_lock((void*)(i<<12));
        page_alloc_data.last_free_idx = i;
        
        // Spin-unlock here
        return (void*)(i<<12);
    }

    // Spin-unlock here
    return NULL;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
/* GLOBAL HELPER FUNCTIONS */
///////////////////////////////////////////////////
///////////////////////////////////////////////////

const u32 mmu_get_reserved_mem()
{
    return page_alloc_data.reserved_mem;
}

const u32 mmu_get_used_mem()
{
    return page_alloc_data.used_mem;
}

const u32 mmu_get_free_mem()
{
    return page_alloc_data.free_mem;
}

bitmap_t const* page_get_bitmap()
{
    return &page_alloc_data.page_bitmap;
}