#include "page_allocator.h"
#include "../memory/memory.h"
#include "../stdio.h"

bitmap_t page_bitmap;
volatile u32 free_mem=0, reserved_mem=0, used_mem=0;
volatile u32 last_free_idx=0;

u32 page_convert_from_bytes(u32 size_bytes)
{
    u32 page_count = size_bytes >> 12;
    if (size_bytes & 0xFFF) page_count++;
    return page_count;
}

void page_alloc_reserven(u32 address, u32 page_count);

void page_alloc_init()
{
    u32 mem_size = memory_get_total_size_bytes();
    u32 bmp_size_bits = page_convert_from_bytes(mem_size);

    bitmap_init(&page_bitmap, bmp_size_bits);
    
    u32 bmp_page_cnt = page_convert_from_bytes(0x20000);
    page_alloc_lockn((u32)page_bitmap.buffer, bmp_page_cnt);
    free_mem = mem_size;

    u32 ent_cnt = mem_map->entries_count;
    for (u32 i=0;i<ent_cnt;i++)
    {
        if (mem_map->regions[i].type != MEMORY_TYPE_FREE)
        {
            u32 num_pages = page_convert_from_bytes(mem_map->regions[i].length);
            page_alloc_reserven(mem_map->regions[i].base, num_pages);
        }
    }
    // for (u32 i=0;i<bmp_size_bits;i++)
    //     kprintf("%u", bitmap_get_bits(&page_bitmap, i));
}

void page_alloc_free(u32 address)
{
    u32 idx = address >> 12; // divide by 4096, which is 1 page
    if (bitmap_get_bits(&page_bitmap, idx)==0) return;
    bitmap_clear_bits(&page_bitmap, idx);
    free_mem+=4096;
    used_mem-=4096;
}

void page_alloc_lock(u32 address)
{
    u32 idx = address >> 12; // divide by 4096, which is 1 page
    if (bitmap_get_bits(&page_bitmap, idx)==1) return;
    bitmap_set_bits(&page_bitmap, idx);
    free_mem-=4096;
    used_mem+=4096;
}

void page_alloc_freen(u32 address, u32 page_count)
{
    for (u32 i=0;i<page_count;i++)
    {
        u32 addr = address+(i<<12);
        page_alloc_free(addr);
    }
}

void page_alloc_lockn(u32 address, u32 page_count)
{
    for (u32 i=0;i<page_count;i++)
    {
        u32 addr = address+(i<<12);
        page_alloc_lock(addr);
    }
}

void page_alloc_reserve(u32 address)
{
    u32 idx = address >> 12; // divide by 4096, which is 1 page
    if (bitmap_get_bits(&page_bitmap, idx)==1) return;
    bitmap_set_bits(&page_bitmap, idx);
    free_mem-=4096;
    reserved_mem+=4096;
}

void page_alloc_reserven(u32 address, u32 page_count)
{
    for (u32 i=0;i<page_count;i++)
    {
        u32 addr = address+(i<<12);
        page_alloc_reserve(addr);
    }
}


const u32 page_get_reserved_mem()
{
    return reserved_mem;
}

const u32 page_get_used_mem()
{
    return used_mem;
}

const u32 page_get_free_mem()
{
    return free_mem;
}

u32 page_alloc_request()
{
    u32 bitmap_bit_count = page_bitmap.size<<3;
    for (u32 i=last_free_idx;i<bitmap_bit_count;i++)
    {
        if (bitmap_get_bits(&page_bitmap, i)==1) continue;

        page_alloc_lock(i<<12);
        last_free_idx = i;
        return i<<12;
    }
    return 0;
}