#include <kernel/mmu.h>
#include <kernel/arch/i686/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/debug.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static struct
{
    u32* page_directory;
    paddr_t page_directory_phys;
} mmu_non_pae_data;

extern bool full_paging_on;

#define PT_ADDR_MASK (~(PAGE_SIZE-1))
#define PAGE_ADDR_MASK (~(PAGE_SIZE-1))
#define HUGE_PAGE_ADDR_MASK (~(HUGE_PAGE_SIZE_NO_PAE-1))

#define PD        ((full_paging_on)?\
((u32*)0xFFFFF000):\
((u32*)mmu_non_pae_data.page_directory))

#define PT_BASE   0xFFC00000
#define PT(i)     ((full_paging_on)?\
((u32*)(PT_BASE + ((i) << 12))):\
((mmu_non_pae_data.page_directory[i]&MMU_PG_ATTR_PRESENT)?\
((u32*)(mmu_non_pae_data.page_directory[i]&PT_ADDR_MASK)):\
NULL))

#define PAGE_INDEX(x) (((x) >> 12) & 0x3FF)
#define PD_INDEX(x)   (((x) >> 22) & 0x3FF)

void __attribute__((cdecl)) mmu_flush_tlb(vaddr_t vaddr);

paddr_t mmu_walk_page_table_non_pae(vaddr_t vaddr);

int mmu_map_page_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes)
{
    // The page directory is an array of 1024 entries
    // Each of these entries is one complete page table
    // Similarly, each of the page tables consists of 1024 entries
    // Each of these entries is one complete page

    // Therefore, to find the page entry index of a page-aligned address,
    // Divide the virtual address by 4096 (shift right by 12) and cap it between 0-1023

    // Same for page table, as one page table is 1024 pages, with a size of
    // 1024*4096=4194304 bytes in total, to find its page table index,
    // Divide the address by the mentioned size (shift right by 22) and cap between 0-1023
    
    u32 pd_index = PD_INDEX(vaddr);
    u32 page_index = PAGE_INDEX(vaddr);
    
    u32* page_table;
    u32 pd_entry = PD[pd_index]; // Obtain page table at index pd_index
    
    page_table = PT(pd_index);
    if (!page_table)
    {
        // Page table does not exist, so allocate a frame, and populate the entry.

        uptr page_table_phys = mmu_frame_alloc(1);
        if (!page_table_phys)
        {
            kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Out of Memory!\n");
            return -1;
        }
        
        pd_entry = (u32)page_table_phys & PT_ADDR_MASK;
        pd_entry |= MMU_PG_ATTR_PRESENT;
        pd_entry |= attributes;
        
        PD[pd_index] = pd_entry;
        page_table = PT(pd_index);
        memset(page_table, 0, PAGE_SIZE);
    }

    // Now that we got the needed page table, obtain the page entry there.
    u32 p_entry = page_table[page_index];
    p_entry = paddr & PAGE_ADDR_MASK;
    p_entry |= MMU_PG_ATTR_PRESENT;
    p_entry |= attributes;
    
    // Reassign
    page_table[page_index] = p_entry;
    return 0;
}

int mmu_map_page_huge_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes)
{
    // Just like with mapping regular 4KB pages, we will start with the page directory first.
    // The difference is, this time, we don't go deeper to page table-page entry level

    // As a result, that makes the whole mapped page 4MB (4KB per what-supposed-to-be page table * 1024 entries)

    u32 huge_page_index = PD_INDEX(vaddr);

    u32 huge_page_entry = PD[huge_page_index];

    huge_page_entry = (paddr_t)(paddr & HUGE_PAGE_ADDR_MASK);
    huge_page_entry |= (MMU_PG_ATTR_PRESENT | MMU_PG_ATTR_PSE);
    huge_page_entry |= attributes;

    PD[huge_page_index] = huge_page_entry;
    return 0;
}

int mmu_unmap_page_non_pae(vaddr_t vaddr)
{
    u32 page_index = PAGE_INDEX(vaddr);
    u32 pd_index = PD_INDEX(vaddr);
    
    // Walk page directory
    u32 pd_entry = PD[pd_index];

    if (!(pd_entry & MMU_PG_ATTR_PRESENT))
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Page NOT exist in page directory!\n");
        return -1;
    }

    // Here, if the address points to a huge page, stop walking.
    if (pd_entry & MMU_PG_ATTR_PSE)
    {
        pd_entry &= ~MMU_PG_ATTR_PRESENT;
        PD[pd_index] = pd_entry;

        goto done;
    }

    // Otherwise, keep walking the page table.
    u32* page_table = PT(pd_index);

    u32 page_entry = page_table[page_index];
    if (!(page_entry & MMU_PG_ATTR_PRESENT))
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Page NOT exist in page table!\n");
        return -1;
    }
    
    page_entry &= ~MMU_PG_ATTR_PRESENT;
    page_table[page_index] = page_entry;

done:
    mmu_flush_tlb(vaddr);
    return 0;
}

paddr_t mmu_walk_page_table_non_pae(vaddr_t vaddr)
{
    u32 page_index = PAGE_INDEX(vaddr);
    u32 pd_index = PD_INDEX(vaddr);
    
    // Walk page directory
    u32 pd_entry = PD[pd_index];

    if (!(pd_entry & MMU_PG_ATTR_PRESENT))
    {
        return 0;
    }

    // Here, if the address points to a huge page, stop walking.
    if (pd_entry & MMU_PG_ATTR_PSE)
    {
        return (paddr_t)(pd_entry & PT_ADDR_MASK);
    }

    // Otherwise, keep walking the page table.
    u32* page_table = PT(pd_index);

    u32 page_entry = page_table[page_index];

    return (paddr_t)(page_entry & PAGE_ADDR_MASK);
}

void mmu_non_pae_init(paddr_t page_dir_addr)
{
    // As features would have been enabled in the mmu_init() call,
    // We will only focus on assigning page directory

    mmu_non_pae_data.page_directory_phys = page_dir_addr;
    mmu_non_pae_data.page_directory = (u32*)(mmu_ptov((paddr_t)page_dir_addr));
    memset(mmu_non_pae_data.page_directory, 0, PAGE_SIZE);

    mmu_non_pae_data.page_directory[1023] = mmu_non_pae_data.page_directory_phys | MMU_PG_ATTR_PRESENT | MMU_PG_ATTR_RW; 
    kdebugf(DEBUG_INFO, "MMU", "Page directory: 0x%llx\n", mmu_non_pae_data.page_directory_phys);
}

void mmu_non_pae_load_page_dir()
{
    mmu_load_address_space(mmu_non_pae_data.page_directory_phys);
}
