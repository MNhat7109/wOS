#include "memory.h"
#include "../stdio.h"

memory_info_t* mem_map;

static const char* const mem_types[] =
{
    "Free",
    "Reserved",
    "ACPI-reclaimable",
    "ACPI non-volatile",
    "Bad",
};

void memory_reserve_boot();
void memory_init(memory_info_t* mem_info)
{
    mem_map = mem_info;
    memory_reserve_boot();
    memory_create_region(0xA0000, 0xF0000-0xA0000, 2);
    memory_merge_region();
}

void memory_reserve_boot()
{
    mem_map->regions[0].type = MEMORY_TYPE_RESERVED;
}

void memory_view_map()
{
    u32 ent_count = mem_map->entries_count;
    kprintf("memory: MEMORY MAP\n");
    for (u32 i=0;i<ent_count;i++)
    {
        kprintf("Region no. %u, Memory type: %s, Start address: 0x%llx, Length: %llu bytes\n",
        i+1, mem_types[mem_map->regions[i].type-1], mem_map->regions[i].base, mem_map->regions[i].length);
    }
    kprintf("\n");
}

void memory_merge_region()
{
    u32 newcnt=0;
    u32 ent_count = mem_map->entries_count;
    memory_region_t new_regions[ent_count];
    for (u32 i=0;i<ent_count-1;)
    {
        new_regions[newcnt].base = mem_map->regions[i].base;
        new_regions[newcnt].length = mem_map->regions[i].length;
        new_regions[newcnt].type = mem_map->regions[i].type;
        new_regions[newcnt].acpi = mem_map->regions[i].acpi;
        u32 j;
        for (j=i+1;j<ent_count;j++)
        {
            if (new_regions[newcnt].type != mem_map->regions[j].type)
            break;
            new_regions[newcnt].length+=mem_map->regions[j].length;
        }
        i=j;
        newcnt++;
    }
    
    for (u32 i=0;i<newcnt;i++)
        mem_map->regions[i] = new_regions[i];
    mem_map->entries_count = newcnt;
}

u32 memory_get_total_size_bytes()
{
    u32 total_size = 0;
    u32 ent_count = mem_map->entries_count;
    for (u32 i=0; i<ent_count; i++)
    {
        total_size += mem_map->regions[i].length;
    }

    return total_size;
}

void swap(memory_region_t* a, memory_region_t* b)
{
    memory_region_t c= *a;
    *a = *b;
    *b = c;
}

u32 part(memory_region_t regions[], u32 lo ,u32 hi)
{
    memory_region_t pivot = regions[hi];
    u32 sto = lo-1;
    for (u32 i = lo; i<hi;i++)
    {
        if (regions[i].base <= pivot.base)
        {
            sto++;
            swap(&regions[sto], &regions[i]);
        }
    }
    swap(&regions[sto+1], &regions[hi]);
    return (sto+1);
}

void qsort(memory_region_t regions[], u32 lo, u32 hi)
{
    if (lo >= hi) return;
    u32 p = part(regions, lo, hi);
    qsort(regions, lo, p-1);
    qsort(regions, p+1, hi);
}

void memory_sort_regions()
{
    qsort(mem_map->regions, 0, mem_map->entries_count-1);
}

void memory_create_region(u64 base, u64 length, u32 type)
{
    memory_region_t region = {
        .base = base,
        .length = length,
        .acpi = 0,
        .type = type
    };

    mem_map->regions[mem_map->entries_count++] = region;
    
    memory_sort_regions();
}