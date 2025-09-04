#include <libk/mmu/mmu.h>
#include <libk/stdio.h>
#include <libk/stdint.h>

memory_info_t* mem_map;

static const char* const mem_types[] =
{
    "Free",
    "Reserved",
    "ACPI-reclaimable",
    "ACPI non-volatile",
    "Bad",
};

static void swap(memory_region_t* a, memory_region_t* b);
static u32 part(memory_region_t regions[], u32 lo ,u32 hi);
static void qsort(memory_region_t regions[], u32 lo, u32 hi);

void mmu_init_map(memory_info_t* mem_info)
{
    mem_map = mem_info;
    mmu_merge_region();
}

void mmu_view_map()
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

void mmu_sort_regions()
{
    qsort(mem_map->regions, 0, mem_map->entries_count-1);
}

void mmu_create_region(u64 base, u64 length, u32 type)
{
    memory_region_t region = {
        .base = base,
        .length = length,
        .acpi = 0,
        .type = type
    };

    mem_map->regions[mem_map->entries_count++] = region;
    
    mmu_sort_regions();
}

void mmu_merge_region()
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

u64 mmu_byte_to_page_count(u64 size_bytes)
{
    u32 page_count = size_bytes >> 12;
    if (size_bytes & 0xFFF) page_count++;
    return page_count;
}

u64 mmu_get_memory_size()
{
    u32 total_size = 0;
    u32 ent_count = mem_map->entries_count;
    for (u32 i=0; i<ent_count; i++)
    {
        total_size += mem_map->regions[i].length;
    }

    return total_size;
}

void* mmu_search_memrange(
    usize paddr_start, usize size, 
    usize step, u64 flags,
    bool (*criterion)(usize paddr)
)
{
    // Identity map from paddr_start...paddr_start+(page_count-1)*PAGE_SIZE
    u64 page_count = mmu_byte_to_page_count(size);
    mmu_mmapn(paddr_start, 0, page_count, 0); // Map as read only
    usize match_addr = 0;

    for (usize i=paddr_start; i<paddr_start+page_count*MMU_PAGE_SIZE; i+=step)
    {
        if (criterion(i))
        {
            match_addr = i;
            break;
        }
    }

    // Now unmap the search region, but map the matched address
    mmu_munmapn(paddr_start, &page_count);
    if (match_addr)
    {
        // Align to page boundary to make sure
        usize page_base = match_addr & ~(MMU_PAGE_SIZE-1);
        mmu_mmap(page_base, page_base, flags);
    }

    return match_addr? (void*)match_addr: NULL;
}

memory_info_t* mmu_get_memory_map()
{
    return mem_map;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

static void swap(memory_region_t* a, memory_region_t* b)
{
    memory_region_t c= *a;
    *a = *b;
    *b = c;
}

static u32 part(memory_region_t regions[], u32 lo ,u32 hi)
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

static void qsort(memory_region_t regions[], u32 lo, u32 hi)
{
    if (lo >= hi) return;
    u32 p = part(regions, lo, hi);
    qsort(regions, lo, p-1);
    qsort(regions, p+1, hi);
}