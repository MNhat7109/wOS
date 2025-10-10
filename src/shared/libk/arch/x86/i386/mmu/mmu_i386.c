#include <libk/mmu/mmu.h>
#include <libk/mmu/mmu_utils.h>
#include <libk/mmu/mmu_paging.h>
#include <libk/mmu/mmu_frame.h>

#include <libk/stdint.h>
#include <libk/string.h>
#include <stdbool.h>

// NOTE: To future devs: Take a break. 
// Otherwise you'll be like Terry Davis 
// (on his schizophrenia part) after like 5 hours 
// looking into those mapping functions.

#ifdef __i386__

#include <arch/x86/i386/paging.h>

#define MMU_PAE_PDPE_ADDRMASK 0x000FFFFFE0000000ULL
#define MMU_PAE_PDE_ADDRMASK  0x000FFFFFFFF00000ULL
#define MMU_PAE_PTE_ADDRMASK     0x000FFFFFFFFFF000ULL

static void mmu_mmap_non_pae(u32 vaddr, u32 paddr, u32 attributes);
static void mmu_mmap_pae(u32 vaddr, u64 paddr, u64 attributes);

static void mmu_mmap_huge_non_pae(u32 vaddr, u32 paddr, u32 attributes);
static void mmu_mmap_huge2m_pae(u32 vaddr, u64 paddr, u64 attributes);
static void mmu_mmap_huge1g_pae(u32 vaddr, u64 paddr, u64 attributes);

static u32 mmu_munmap_non_pae(u32 vaddr);
static u64 mmu_munmap_pae(u32 vaddr);


void mmu_mmap(u64 vaddr, u64 paddr, u64 attributes)
{
    u32 features = mmu_paging_get_features();
    if (features & MMU_PAGING_PAE)
        mmu_mmap_pae(vaddr, paddr, attributes);
    else
        mmu_mmap_non_pae(vaddr, paddr, attributes);

    if (features & MMU_PAGING_PRESENT)
        i386_tlb_flush(vaddr);
}

void mmu_mmapn(u64 addr, u64 offset, u64 n, u64 attributes)
{
    if (!n) return;
    offset &= ~0xFFFULL;
    u64 page_size = mmu_get_page_size();

    for (u32 i=0;i<n;i++)
        mmu_mmap(addr+offset+i*page_size, addr+i*page_size, attributes);
}

u64 mmu_munmap(u64 vaddr)
{
    u32 features = mmu_paging_get_features();
    u64 base;
    if (features & MMU_PAGING_PAE)
        base = mmu_munmap_pae(vaddr);
    else base = mmu_munmap_non_pae(vaddr);

    if (!base) return 0;

    if (features & MMU_PAGING_PRESENT)
        i386_tlb_flush(vaddr);

    return base;
}

u64 mmu_munmapn(u64 vaddr, u64* n)
{
    if (!n || *n == 0) return 0;

    u64 page_size = mmu_get_page_size();
    u64 unmapped = 0;
    u64 paddr = mmu_munmap(vaddr);
    if (!paddr) 
    {
        *n = unmapped;
        return 0;
    }
    unmapped++;

    for (u32 i=1;i<*n;i++)
    {
        u64 paddr_i = mmu_munmap(vaddr+i*page_size);
        if (!paddr_i)
        {
            *n = unmapped;
            break;
        }
        unmapped++;
    }

    return paddr;
}

void mmu_mmap_huge(u64 vaddr, u64 paddr, u64 attributes, int flags)
{
    u32 features = mmu_paging_get_features();
    if (features & MMU_PAGING_PAE)
    {
        if (flags & MMU_HUGE_PAGE_2M)
        mmu_mmap_huge2m_pae(vaddr, paddr, attributes);
        else if (flags & MMU_HUGE_PAGE_1G)
        mmu_mmap_huge1g_pae(vaddr, paddr, attributes);
    }
    else
        mmu_mmap_huge_non_pae(vaddr, paddr, attributes);

    if (features & MMU_PAGING_PRESENT)
        i386_tlb_flush(vaddr);
}

void mmu_mmapn_huge(u64 addr, u64 offset, u64 n, u64 attributes, int flags)
{
    if (!n) return;
    offset &= ~0xFFFULL;
    u64 page_size = mmu_get_huge_page_size(flags);

    for (u32 i=0;i<n;i++)
        mmu_mmap_huge(addr+offset+i*page_size, addr+i*page_size, attributes, flags);
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

static void mmu_mmap_non_pae(u32 vaddr, u32 paddr, u32 attributes)
{
    u32* page_dir = (u32*)mmu_paging_get_addr();
    usize page_size = mmu_get_page_size();

    u32 page_index = (vaddr >> 12)&0x3FF;
    u32 pt_index = (vaddr >> 22)&0x3FF;
    
    u32* page_table;
    u32 pd_entry = page_dir[pt_index];

    if (pd_entry & MMU_PA_FLAG_PRESENT)
    {
        page_table = (u32*)(pd_entry & 0xFFFFF000); // Make it page-aligned
    }
    else
    {
        u64 frame_phys = mmu_frame_create(page_size);
        page_table = (u32*)mmu_ptov(frame_phys);
        memset(page_table, 0, page_size);

        pd_entry = (u32)page_table & 0xFFFFF000;
        pd_entry |= MMU_PA_FLAG_PRESENT;
        pd_entry |= attributes;

        page_dir[pt_index] = pd_entry;
    }

    u32 p_entry = page_table[page_index];
    p_entry = paddr & 0xFFFFF000; // Page-aligned
    p_entry |= MMU_PA_FLAG_PRESENT;
    p_entry |= (u32)attributes;

    page_table[page_index] = p_entry;
}

static void mmu_mmap_pae(u32 vaddr, u64 paddr, u64 attributes)
{
    u64* page_dir_ptr = (u64*)mmu_paging_get_addr();
    usize page_size = mmu_get_page_size();

    u64 pd_index = (vaddr >> 30)&0x3;
    u64 pt_index = (vaddr >> 21)&0x1FF;
    u64 page_index = (vaddr >> 12)&0x1FF;

    u64* page_dir;
    u64 pdp_entry = page_dir_ptr[pd_index];

    if (pdp_entry & MMU_PA_FLAG_PRESENT)
    page_dir = (u64*)(pdp_entry & MMU_PAE_PDPE_ADDRMASK);
    else
    {
        u64 pd_frame_phys = mmu_frame_create(page_size);
        page_dir = (u64*)mmu_ptov(pd_frame_phys);
        memset(page_dir, 0, page_size);

        pdp_entry = (u64)page_dir & MMU_PAE_PDPE_ADDRMASK;
        pdp_entry |= MMU_PA_FLAG_PRESENT;
        pdp_entry |= attributes;

        page_dir_ptr[pd_index] = pdp_entry;
    }

    u64* page_table;
    u64 pd_entry = page_dir[pt_index];

    if (pd_entry & MMU_PA_FLAG_PRESENT)
        page_table = (u64*)(pd_entry & MMU_PAE_PDE_ADDRMASK);
    else
    {
        u64 pt_frame_phys = mmu_frame_create(page_size);
        page_table = (u64*)mmu_ptov(pt_frame_phys);
        memset(page_table, 0, page_size);

        pd_entry = (u64)page_table & MMU_PAE_PDE_ADDRMASK;
        pd_entry |= MMU_PA_FLAG_PRESENT;
        pd_entry |= attributes;

        page_dir[pt_index] = pd_entry;
    }

    u64 p_entry = page_table[page_index];
    p_entry = paddr & MMU_PAE_PTE_ADDRMASK;
    p_entry |= MMU_PA_FLAG_PRESENT;
    p_entry |= attributes;

    page_table[page_index] = p_entry;
}

static void mmu_mmap_huge_non_pae(u32 vaddr, u32 paddr, u32 attributes)
{
    u32* page_dir = (u32*)mmu_paging_get_addr();

    u32 page_index = (vaddr >> 22)&0x3FF;
    
    u32 p_entry = page_dir[page_index];
    p_entry = paddr & ~0x3FFFFFUL; // 4 MiB-aligned
    p_entry |= MMU_PA_FLAG_PRESENT;
    p_entry |= MMU_PA_FLAG_PSE;
    p_entry |= attributes;

    page_dir[page_index] = p_entry;
}

static void mmu_mmap_huge2m_pae(u32 vaddr, u64 paddr, u64 attributes)
{
    u64* page_dir_ptr = (u64*)mmu_paging_get_addr();
    usize page_size = mmu_get_page_size();

    u64 pd_index = (vaddr >> 30)&0x3;
    u64 page_index = (vaddr >> 21)&0x1FF;

    u64* page_dir;
    u64 pdp_entry = page_dir_ptr[pd_index];

    if (pdp_entry & MMU_PA_FLAG_PRESENT)
        page_dir = (u64*)(pdp_entry & MMU_PAE_PDPE_ADDRMASK);
    else
    {
        u64 pd_frame_phys = mmu_frame_create(page_size);
        page_dir = (u64*)mmu_ptov(pd_frame_phys);
        memset(page_dir, 0, page_size);

        pdp_entry = (u64)page_dir & MMU_PAE_PDPE_ADDRMASK;
        pdp_entry |= MMU_PA_FLAG_PRESENT;
        pdp_entry |= attributes;

        page_dir_ptr[pd_index] = pdp_entry;
    }

    u64 p_entry = page_dir[page_index];
    p_entry = paddr & MMU_PAE_PDE_ADDRMASK;
    p_entry |= MMU_PA_FLAG_PRESENT;
    p_entry |= MMU_PA_FLAG_PSE;
    p_entry |= attributes;

    page_dir[page_index] = p_entry;
}

static void mmu_mmap_huge1g_pae(u32 vaddr, u64 paddr, u64 attributes)
{
    u64* page_dir_ptr = (u64*)mmu_paging_get_addr();
    usize page_size = mmu_get_page_size();

    u64 page_index = (vaddr >> 30)&0x3;
    
    u64 p_entry = page_dir_ptr[page_index];
    p_entry = paddr & MMU_PAE_PDPE_ADDRMASK;
    p_entry |= MMU_PA_FLAG_PRESENT;
    p_entry |= MMU_PA_FLAG_PSE;
    p_entry |= attributes;

    page_dir_ptr[page_index] = p_entry;
}

#endif