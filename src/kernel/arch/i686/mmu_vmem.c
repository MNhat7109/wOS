#include <kernel/mmu_vmem.h>
#include <kernel/arch/i686/mmu.h>

usize HUGE_PAGE_SIZE();

int mmu_vmem_interpret_flags(int flags)
{
    int page_attributes = 0;
    if ((flags & MMU_VMA_UC)) page_attributes |= MMU_PG_ATTR_PCD;
    if (((flags&MMU_VMA_R)&&(flags&MMU_VMA_W))
    || (flags&MMU_VMA_W)) page_attributes |= MMU_PG_ATTR_RW;
    return page_attributes;
}

usize mmu_vmem_get_page_size(int flags)
{
    if ((flags & MMU_VMA_HUGE_PAGE)) return HUGE_PAGE_SIZE();
    return PAGE_SIZE;
}