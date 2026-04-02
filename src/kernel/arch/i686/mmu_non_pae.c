#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/debug.h>
#include <string.h>
#include <stdint.h>

static struct
{
    u32* page_directory;
    paddr_t page_directory_phys;
} mmu_non_pae_data;

#define PT_ADDR_MASK (~(PAGE_SIZE-1))
#define PAGE_ADDR_MASK (~(PAGE_SIZE-1))
#define HUGE_PAGE_ADDR_MASK (~(HUGE_PAGE_SIZE_NO_PAE-1))

void __attribute__((cdecl)) mmu_flush_tlb(vaddr_t vaddr);

void mmu_mmap_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes)
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
    
    u32 page_index = (vaddr >> 12)&0x3FF;
    u32 pt_index = (vaddr >> 22)&0x3FF;

    u32* page_table;
    u32 pd_entry = mmu_non_pae_data.page_directory[pt_index]; // Obtain page table at index pt_index

    if (pd_entry & MMU_PG_ATTR_PRESENT)
    {
        // Page table exists. 
        // Take the upper 20 bits of the entry (which is the page-aligned address of the page table)
        
        page_table = (u32*)(pd_entry & PT_ADDR_MASK);
    }
    else
    {
        // Page table does not exist, so allocate a frame, and populate the entry.

        uptr phys_frame = mmu_frame_request(PAGE_SIZE);
        page_table = (u32*)mmu_ptov((paddr_t)phys_frame);
        memset(page_table, 0, PAGE_SIZE);
        
        pd_entry = (u32)phys_frame & PT_ADDR_MASK;
        pd_entry |= MMU_PG_ATTR_PRESENT;
        pd_entry |= attributes;

        mmu_non_pae_data.page_directory[pt_index] = pd_entry;
    }

    // Now that we got the needed page table, obtain the page entry there.
    u32 p_entry = page_table[page_index];
    p_entry = paddr & PAGE_ADDR_MASK;
    p_entry |= MMU_PG_ATTR_PRESENT;
    p_entry |= attributes;

    // Reassign
    page_table[page_index] = p_entry;
}

void mmu_mmap_huge_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes)
{
    // Just like with mapping regular 4KB pages, we will start with the page directory first.
    // The difference is, this time, we don't go deeper to page table-page entry level

    // As a result, that makes the whole mapped page 4MB (4KB per what-supposed-to-be page table * 1024 entries)

    u32 huge_page_index = (vaddr >> 22)&0x3FF;

    u32 huge_page_entry = mmu_non_pae_data.page_directory[huge_page_index];

    huge_page_entry = (paddr_t)(paddr & HUGE_PAGE_ADDR_MASK);
    huge_page_entry |= (MMU_PG_ATTR_PRESENT | MMU_PG_ATTR_PSE);
    huge_page_entry |= attributes;

    mmu_non_pae_data.page_directory[huge_page_index] = huge_page_entry;
}

usize mmu_munmap_non_pae(vaddr_t vaddr, paddr_t* paddr_out)
{
    u32 page_index = (vaddr >> 12)&0x3FF;
    u32 pt_index = (vaddr >> 22)&0x3FF;
    usize return_size =0;
    
    // Walk page directory
    u32 pt_entry = mmu_non_pae_data.page_directory[pt_index];

    if (!(pt_entry & MMU_PG_ATTR_PRESENT))
    {
        *paddr_out = 0;
        return 0;
    }

    // Here, if the address points to a huge page, stop walking.
    if (pt_entry & MMU_PG_ATTR_PSE)
    {
        pt_entry &= ~MMU_PG_ATTR_PRESENT;
        *paddr_out = (paddr_t)(pt_entry & PT_ADDR_MASK);
        mmu_non_pae_data.page_directory[pt_index] = pt_entry;

        return_size = HUGE_PAGE_SIZE_NO_PAE;
        goto done;
    }

    // Otherwise, keep walking the page table.
    u32* page_table = (u32*)(pt_entry & PT_ADDR_MASK);

    u32 page_entry = page_table[page_index];
    if (!(page_entry & MMU_PG_ATTR_PRESENT))
    {
        *paddr_out = 0;
        return 0;
    }

    page_entry &= ~MMU_PG_ATTR_PRESENT;
    *paddr_out = (paddr_t)(page_entry & PAGE_ADDR_MASK);
    page_table[page_index] = page_entry;
    return_size = PAGE_SIZE;

done:
    mmu_flush_tlb(vaddr);
    return return_size;
}

void mmu_non_pae_init(paddr_t page_dir_addr)
{
    // As features would have been enabled in the mmu_init() call,
    // We will only focus on assigning page directory

    mmu_non_pae_data.page_directory_phys = page_dir_addr;
    mmu_non_pae_data.page_directory = (u32*)(mmu_ptov((paddr_t)page_dir_addr));
    memset(mmu_non_pae_data.page_directory, 0, PAGE_SIZE);

    kdebugf(DEBUG_INFO, "MMU", "Page directory: 0x%llx\n", mmu_non_pae_data.page_directory_phys);
}

void mmu_non_pae_load_page_dir()
{
    mmu_load_address_space(mmu_non_pae_data.page_directory_phys);
}