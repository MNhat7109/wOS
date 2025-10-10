#include <libk/mmu/mmu.h>
#include <libk/mmu/mmu_map.h>
#include <libk/mmu/mmu_utils.h>
#include <libk/mmu/mmu_frame.h>
#include <libk/mmu/mmu_paging.h>

#include <libk/bitmanip/bitmanip.h>
#include <libk/stdio.h>
#include <libk/stdint.h>

usize vaddr_offset = 0;

u64 mmu_get_total_memsize()
{
    memory_info_t* mem_map = mmu_map_get();

    u64 total_size = 0;
    u32 ent_count = mem_map->entries_count;
    for (u32 i=0; i<ent_count; i++)
    {
        total_size += mem_map->regions[i].length;
    }

    return total_size;
}

usize mmu_get_reserved_memsize()
{
    usize sum = 0;
    for (int i=MMU_FRAME_TYPE_RESERVED; i<MMU_FRAME_TYPE_OTHER; i++)
    {
        sum+=mmu_frame_get_size(i);
    }

    return sum;
}

usize mmu_get_free_memsize()
{
    return mmu_frame_get_size(MMU_FRAME_TYPE_FREE);
}

usize mmu_get_used_memsize()
{
    return mmu_frame_get_size(MMU_FRAME_TYPE_USED);
}

usize mmu_log2(usize n)
{
    return (sizeof(n)-1) - clz(n);
}

usize mmu_ptov(usize paddr)
{
    u32 features = mmu_paging_get_features();

    if (features & MMU_PAGING_PRESENT)
        return paddr + vaddr_offset;
    return paddr;
}

usize mmu_vtop(usize vaddr)
{
    u32 features = mmu_paging_get_features();

    if (features & MMU_PAGING_PRESENT)
        return vaddr - vaddr_offset;
    return vaddr;
}