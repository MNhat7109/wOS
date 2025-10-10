#include <libk/mmu/mmu.h>
#include <libk/mmu/mmu_paging.h>

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
/* GLOBAL HELPER FUNCTIONS */
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

u64 mmu_search_memrange(
    usize paddr_start, usize size, 
    usize step, u64 flags,
    bool (*criterion)(usize paddr)
)
{
    // Identity map from paddr_start...paddr_start+(page_count-1)*PAGE_SIZE
    u64 page_count = mmu_byte_to_4k_pages(size);
    u32 page_size = mmu_get_page_size();
    mmu_mmapn(paddr_start, 0, page_count, 0); // Map as read only
    usize match_addr = 0;

    for (usize i=paddr_start; i<paddr_start+page_count*page_size; i+=step)
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
        usize page_base = match_addr & ~(page_size-1);
        mmu_mmap(page_base, page_base, flags);
    }

    return match_addr? (void*)match_addr: NULL;
}

u32 mmu_get_page_size()
{
    return (1<<12);
}

u32 mmu_get_huge_page_size(int flags)
{
    u32 features = mmu_paging_get_features();
    if (features & MMU_PAGING_PAE)
    {
        if (flags & MMU_HUGE_PAGE_1G) return (1<<30);
        return (1<<21);
    }
    return (1<<22);
}