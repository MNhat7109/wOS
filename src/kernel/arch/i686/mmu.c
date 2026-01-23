#include <kernel/mmu.h>
#include <kernel/debug.h>
#include <stdbool.h>

#define HIGHER_HALF_OFFSET 0xC0000000

static struct
{
    struct
    {
        bool pae_enable : 1;
        bool nx_enable : 1;
        bool pse_enable : 1;
        u8 _padding : 5;
    } __attribute__((packed)) features;
} mmu_data;

void mmu_non_pae_init(paddr_t page_dir_addr);
void mmu_non_pae_page_dir_recalibrate();
void mmu_non_pae_load_page_dir();

void __attribute__((cdecl)) mmu_load_address_space_i686(paddr_t paddr);
void __attribute__((cdecl)) mmu_enable_paging();

void mmu_mmap_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
void mmu_mmap_huge_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
usize mmu_munmap_non_pae(vaddr_t vaddr, paddr_t* paddr_out);

void mmu_mmap_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmap_huge_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
usize mmu_munmap_pae(vaddr_t vaddr, paddr_t* paddr_out);

void mmu_arch_init(uptr first_free_page)
{
    kdebugf(DEBUG_INFO, "MMU", "Initializing i686 MMU...\n");
    
    // Check features
    // TODO
    
    // Init based on features
    mmu_non_pae_init((paddr_t)first_free_page);
}

void mmu_load_address_space(paddr_t paddr)
{
    mmu_load_address_space_i686(paddr);
}

void mmu_enable_features()
{
    // TODO: This will be changed as PAE is getting added
    mmu_non_pae_load_page_dir();
    
    // TODO: Enable additional features
    
    mmu_enable_paging();
}

usize HUGE_PAGE_SIZE()
{
    if (!mmu_data.features.pse_enable) return 0;
    return mmu_data.features.pae_enable?HUGE_PAGE_SIZE_PAE:HUGE_PAGE_SIZE_NO_PAE;
}

paddr_t mmu_vtop(vaddr_t vaddr)
{
    return (paddr_t)(vaddr-HIGHER_HALF_OFFSET);
}    

vaddr_t mmu_ptov(paddr_t paddr)
{
    return (vaddr_t)(paddr+HIGHER_HALF_OFFSET);
}    

void mmu_mmap(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    if (mmu_data.features.pae_enable)
    {
        if (!mmu_data.features.nx_enable)
            attributes &= ~MMU_PG_ATTR_NX;
        mmu_mmap_pae(vaddr, paddr, attributes);
        return;
    }
    mmu_mmap_non_pae(vaddr, paddr,attributes);
}

void mmu_mmapn(paddr_t addr, usize n, u64 attributes, int flags)
{
    vaddr_t va;
    if (flags & MMU_FLAG_MAP_ID) va = (vaddr_t)addr;
    else va = mmu_ptov(addr);
    kdebugf(DEBUG_INFO, "MMU", "Mapping 0x%llx, count=%u, va=0x%x\n", addr, n, va);
    
    usize huge_page_size;
    if (flags & MMU_FLAG_HUGE_PAGE) 
    {
        huge_page_size=HUGE_PAGE_SIZE();
        if (huge_page_size == 0) return;
    }
    
    for (usize i=0;i<n;i++)
    {
        if (flags & MMU_FLAG_HUGE_PAGE)
            mmu_mmap_huge(va+i*huge_page_size, addr+i*huge_page_size, attributes);
        else mmu_mmap(va+i*PAGE_SIZE, addr+i*PAGE_SIZE, attributes);
    }
}

void mmu_mmap_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    if (!mmu_data.features.pse_enable) return;

    if (mmu_data.features.pae_enable)
    {
        if (!mmu_data.features.nx_enable)
            attributes &= ~MMU_PG_ATTR_NX;
        mmu_mmap_huge_pae(vaddr, paddr, attributes);
        return;
    }
    mmu_mmap_huge_non_pae(vaddr, paddr,attributes);
}

void mmu_mmap_very_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    // Stub only in i386, will do nothing
    return;
}

usize mmu_munmap(vaddr_t vaddr, paddr_t* paddr_out)
{
    if (mmu_data.features.pae_enable)
    {
        return mmu_munmap_pae(vaddr, paddr_out);
    }
    return mmu_munmap_non_pae(vaddr, paddr_out);
}

usize mmu_munmapn(vaddr_t vaddr, paddr_t* first_paddr_out, usize n)
{
    if (n == 0) return 0;

    paddr_t paddr; u64 unmapped = 0;
    usize page_size = mmu_munmap(vaddr, &paddr);

    if (!paddr)
    {
        *first_paddr_out = 0;
        goto finished;
    }

    unmapped++; vaddr+=page_size;

    while (unmapped < n)
    {
        paddr_t paddr_i;
        page_size = mmu_munmap(vaddr, &paddr_i);

        if (!paddr_i)
            break;

        vaddr+=page_size;
        unmapped++;
    }

finished:
    return unmapped;
}