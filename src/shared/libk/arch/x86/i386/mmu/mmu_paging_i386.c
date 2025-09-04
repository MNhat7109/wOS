#include <libk/stdint.h>
#include <libk/mmu/page_allocator.h>
#include <libk/mmu/mmu.h>
#include <libk/string.h>

// NOTE: To future devs: Take a break. 
// Otherwise you'll be like Terry Davis 
// (his schizophrenia) after like 5 hours 
// looking into those mapping functions.

#ifdef __i386__

#include <arch/x86/i386/paging.h>
#include <stdbool.h>

#define MMU_PAE_ADDRMASK 0x000FFFFFFFFFF000ULL

static struct
{
    bool supports_pae;
    u64* page_dir_ptr;
    u32* page_dir;
} mmu_i386_paging;

static void mmu_mmap_no_pae(u32 vaddr, u32 paddr, u32 flags);
static void mmu_mmap_pae(u32 vaddr, u64 paddr, u64 flags);

static void mmu_mmap_huge_no_pae(u32 vaddr, u32 paddr, u32 flags);
static void mmu_mmap_huge_pae(u32 vaddr, u64 paddr, u64 flags);

static u32 mmu_munmap_no_pae(u32 vaddr);
static u64 mmu_munmap_pae(u32 vaddr);

void mmu_init_pages(void* addr, bool pae)
{
    // Check for MMU support
    mmu_i386_paging.supports_pae = pae;

    if (mmu_i386_paging.supports_pae) 
    mmu_i386_paging.page_dir_ptr = (u64*)addr;
    else 
    mmu_i386_paging.page_dir = (u32*)addr;
}

void mmu_mmap(u64 vaddr, u64 paddr, u64 flags)
{
    if (mmu_i386_paging.supports_pae)
    {
        if (flags & MMU_PT_FLAG_PSE)
            mmu_mmap_huge_pae(vaddr, paddr, flags);
        else mmu_mmap_pae(vaddr, paddr, flags);
    }
    else
    {
        if (flags & MMU_PT_FLAG_PSE)
            mmu_mmap_huge_no_pae(vaddr, paddr, flags);
        else mmu_mmap_no_pae(vaddr, paddr, flags);
    }

    if (mmu_get_status())
    i386_tlb_flush(vaddr);
}

void mmu_mmapn(u64 addr, u64 offset, u64 n, u64 flags)
{
    if (!n) return;
    offset &= ~0xFFFULL;

    for (u32 i=0;i<n;i++)
        mmu_mmap(addr+offset+i*0x1000, addr+i*0x1000, flags);
}

u64 mmu_munmap(u64 vaddr)
{
    u64 base;
    if (mmu_i386_paging.supports_pae)
        base = mmu_munmap_pae(vaddr);
    else base = mmu_munmap_no_pae(vaddr);

    if (!base) return 0;

    if (mmu_get_status())
    i386_tlb_flush(vaddr);

    return base;
}

u64 mmu_munmapn(u64 vaddr, u64* n)
{
    if (!n || *n == 0) return 0;

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
        u64 paddr_i = mmu_munmap(vaddr+i*0x1000);
        if (!paddr_i)
        {
            *n = unmapped;
            break;
        }
        unmapped++;
    }

    return paddr;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

static void mmu_mmap_no_pae(u32 vaddr, u32 paddr, u32 flags)
{
    u32 page_index = (vaddr >> 12)&0x3FF;
    u32 pt_index = (vaddr >> 22)&0x3FF;
    
    u32* page_table;
    u32 pd_entry = mmu_i386_paging.page_dir[pt_index];

    if (pd_entry & MMU_PT_FLAG_PRESENT)
    {
        page_table = (u32*)(pd_entry & 0xFFFFF000); // Make it page-aligned
    }
    else
    {
        page_table = (u32*)page_alloc_request();
        memset(page_table, 0, 0x1000);

        pd_entry = (u32)page_table & 0xFFFFF000;
        pd_entry |= MMU_PT_FLAG_PRESENT;
        pd_entry |= flags;

        mmu_i386_paging.page_dir[pt_index] = pd_entry;
    }

    u32 p_entry = page_table[page_index];
    p_entry = paddr & 0xFFFFF000; // Page-aligned
    p_entry |= MMU_PT_FLAG_PRESENT;
    p_entry |= (u32)flags;

    page_table[page_index] = p_entry;
}

static void mmu_mmap_pae(u32 vaddr, u64 paddr, u64 flags)
{
    u64 pd_index = (vaddr >> 30)&0x3;
    u64 pt_index = (vaddr >> 21)&0x1FF;
    u64 page_index = (vaddr >> 12)&0x1FF;

    
    u64* page_dir;
    u64 pdp_entry = mmu_i386_paging.page_dir_ptr[pd_index];
    if (pdp_entry & MMU_PT_FLAG_PRESENT)
    page_dir = (u64*)(pdp_entry & MMU_PAE_ADDRMASK);
    else
    {
        page_dir = (u64*)page_alloc_request();
        memset(page_dir, 0, 0x1000);

        pdp_entry = (u64)page_dir & MMU_PAE_ADDRMASK;
        pdp_entry |= MMU_PT_FLAG_PRESENT;
        pdp_entry |= flags;

        mmu_i386_paging.page_dir_ptr[pd_index] = pdp_entry;
    }

    u64* page_table;
    u64 pd_entry = page_dir[pt_index];
    if (pd_entry & MMU_PT_FLAG_PRESENT)
        page_table = (u64*)(pd_entry & MMU_PAE_ADDRMASK);
    else
    {
        page_table = (u64*)page_alloc_request();
        memset(page_table, 0, 0x1000);

        pd_entry = (u64)page_table & MMU_PAE_ADDRMASK;
        pd_entry |= MMU_PT_FLAG_PRESENT;
        pd_entry |= flags;

        page_dir[pt_index] = pd_entry;
    }

    u64 p_entry = page_table[page_index];
    p_entry = paddr & MMU_PAE_ADDRMASK;
    p_entry |= MMU_PT_FLAG_PRESENT;
    p_entry |= flags;

    page_table[page_index] = p_entry;
}

static void mmu_mmap_huge_no_pae(u32 vaddr, u32 paddr, u32 flags)
{
    u32 page_index = (vaddr >> 22)&0x3FF;
    
    u32 p_entry = mmu_i386_paging.page_dir[page_index];
    p_entry = paddr & ~0x3FFFFFUL; // 4 MiB-aligned
    p_entry |= MMU_PT_FLAG_PRESENT;
    p_entry |= flags;

    mmu_i386_paging.page_dir[page_index] = p_entry;
}

static void mmu_mmap_huge_pae(u32 vaddr, u64 paddr, u64 flags)
{
    u64 pd_index = (vaddr >> 30)&0x3;
    u64 page_index = (vaddr >> 21)&0x1FF;

    u64* page_dir;
    u64 pdp_entry = mmu_i386_paging.page_dir_ptr[pd_index];
    if (pdp_entry & MMU_PT_FLAG_PRESENT)
        page_dir = (u64*)(pdp_entry & MMU_PAE_ADDRMASK);
    else
    {
        page_dir = (u64*)page_alloc_request();
        memset(page_dir, 0, 0x1000);

        pdp_entry = (u64)page_dir & MMU_PAE_ADDRMASK;
        pdp_entry |= MMU_PT_FLAG_PRESENT;
        pdp_entry |= flags;

        mmu_i386_paging.page_dir_ptr[pd_index] = pdp_entry;
    }

    u64 p_entry = page_dir[page_index];
    p_entry = paddr & 0x000FFFFFFFFFE000ULL;
    p_entry |= MMU_PT_FLAG_PRESENT;
    p_entry |= flags;

    page_dir[page_index] = p_entry;
}

static u32 mmu_munmap_no_pae(u32 vaddr)
{
    u32 paddr;

    u32 page_index = (vaddr >> 12) & 0x3FF; 
    u32 pt_index = (vaddr >> 22) & 0x3FF; 

    u32 pd_entry = mmu_i386_paging.page_dir[pt_index];
    
    if (!(pd_entry & MMU_PT_FLAG_PRESENT)) return 0;
    if (pd_entry & MMU_PT_FLAG_PSE)
    {
        paddr = pd_entry & ~0x3FFFFFUL;

        mmu_i386_paging.page_dir[pt_index] = 0;
        return paddr;
    }

    u32* page_table = (u32*)(pd_entry & 0xFFFFF000);
    u32 page_entry = page_table[page_index];

    if (!(page_entry & MMU_PT_FLAG_PRESENT)) return 0;

    paddr = page_entry & 0xFFFFF000;
    page_table[page_index] = 0;

    return paddr;
}

static u64 mmu_munmap_pae(u32 vaddr)
{
    u64 paddr;

    u64 pd_index = (vaddr >> 30)&0x3;
    u64 pt_index = (vaddr >> 21)&0x1FF;
    u64 page_index = (vaddr >> 12)&0x1FF;

    u64 pdp_entry = mmu_i386_paging.page_dir_ptr[pd_index];
    if (!(pdp_entry & MMU_PT_FLAG_PRESENT)) return 0;

    u64* page_dir = (u64*)(pdp_entry & MMU_PAE_ADDRMASK);
    u64 pd_entry = page_dir[pt_index];

    if (!(pd_entry & MMU_PT_FLAG_PRESENT)) return 0;
    if (pd_entry & MMU_PT_FLAG_PSE)
    {
        paddr = pd_entry & 0x000FFFFFFFFFE000ULL;

        page_dir[pt_index] = 0;
        return paddr;
    }

    u64* page_table = (u64*)(pd_entry & MMU_PAE_ADDRMASK);
    u64 page_entry = page_table[page_index];

    if (!(page_entry & MMU_PT_FLAG_PRESENT)) return 0;

    paddr = page_entry & MMU_PAE_ADDRMASK;
    page_table[page_index] = 0;

    return paddr;
}

#endif