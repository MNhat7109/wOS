#include <kernel/mmu.h>
#include <kernel/debug.h>
#include <kernel/arch/i686/cpuid.h>
#include <stdbool.h>
#include <string.h>

#define HIGHER_HALF_OFFSET 0xC0000000

typedef struct mmu_features_t
{
    bool pae : 1;
    bool nx : 1;
    bool pse : 1;
    u32 _padding : 29;
} __attribute__((packed)) mmu_features_t;

static struct
{
    mmu_features_t cap;
    mmu_features_t present;
    mmu_features_t intended;
} mmu_data;

void mmu_check_features();
void mmu_init_paging(paddr_t page_dir_addr);
void mmu_reload_paging_addr();

void __attribute__((cdecl)) mmu_load_address_space_i686(paddr_t paddr);
void __attribute__((cdecl)) mmu_enable_paging();
void __attribute__((cdecl)) mmu_enable_pse();

void mmu_non_pae_init(paddr_t page_dir_addr);
void mmu_non_pae_load_page_dir();

void mmu_mmap_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
void mmu_mmap_huge_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
usize mmu_munmap_non_pae(vaddr_t vaddr, paddr_t* paddr_out);

void mmu_mmap_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmap_huge_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
usize mmu_munmap_pae(vaddr_t vaddr, paddr_t* paddr_out);

void mmu_arch_init(uptr first_free_page, u32 optional_features)
{
    kdebugf(DEBUG_INFO, "MMU", "Initializing i686 MMU...\n");
    kdebugf(DEBUG_INFO, "MMU", "Probing features...\n");

    // Check additional MMU features
    mmu_check_features();

    // Store additional settings
    memcpy(&mmu_data.intended, &optional_features, sizeof(mmu_data.intended));

    const char* yn[2] = {"no", "yes"};
    kdebugf(DEBUG_INFO, "MMU", "MMU additional features:\n"
    "\tPhysical Address Extension: present: %s, to be toggled: %s\n"
    "\tPage Size Extension: present: %s, to be toggled: %s\n"
    "\tExecute Disable bit: present: %s, to be toggled: %s\n",
    yn[mmu_data.cap.pae],yn[mmu_data.intended.pae],
    yn[mmu_data.cap.pse],yn[mmu_data.intended.pse],
    yn[mmu_data.cap. nx],yn[mmu_data.intended. nx]);

    mmu_data.present.pse = mmu_data.intended.pse && mmu_data.cap.pse;
    mmu_data.present.pae = mmu_data.intended.pae && mmu_data.cap.pae;
    mmu_data.present.nx = mmu_data.intended.nx && mmu_data.cap.nx;

    // Init based on features
    mmu_init_paging((paddr_t)first_free_page);
}

void mmu_load_address_space(paddr_t paddr)
{
    mmu_load_address_space_i686(paddr);
}

void mmu_enable_features()
{
    // TODO: This will be changed as PAE is getting added
    mmu_reload_paging_addr();
    
    // TODO: Enable additional features
    // Enable PSE
    if (mmu_data.present.pse)
        mmu_enable_pse();
    
    mmu_enable_paging();
}

usize HUGE_PAGE_SIZE()
{
    if (!mmu_data.present.pse) return 0;
    return mmu_data.present.pae?HUGE_PAGE_SIZE_PAE:HUGE_PAGE_SIZE_NO_PAE;
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
    if (mmu_data.present.pae)
    {
        if (!mmu_data.present.nx)
            attributes &= ~MMU_PG_ATTR_NX;
        // mmu_mmap_pae(vaddr, paddr, attributes);
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
    if (!mmu_data.present.pse) return;

    if (mmu_data.present.pae)
    {
        if (!mmu_data.present.nx)
            attributes &= ~MMU_PG_ATTR_NX;
        // mmu_mmap_huge_pae(vaddr, paddr, attributes);
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
    if (mmu_data.present.pae)
    {
        // return mmu_munmap_pae(vaddr, paddr_out);
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

void mmu_check_features()
{
    // Check for PSE and PAE
    u32 cpu_info_out[4];
    cpuid(CPUID_FUNC_GETFEATURES, cpu_info_out);
    
    // Bit 3 and 6 of EDX will reveal if PSE and PAE, respectively, are available.
    mmu_data.cap.pse = (cpu_info_out[3] >> 3) & 1;
    mmu_data.cap.pae = (cpu_info_out[3] >> 6) & 1;

    // Now check for NX
    // At this point, if PAE is not available, then we can stop.
    if (!mmu_data.cap.pae) return;

    kdebugf(DEBUG_INFO, "MMU", "Checking for NX...\n");

    // Query max extended leaf
    // Because we must poke at function (leaf) 0x80000001 to check for NX,
    // We must first check how many leaves the CPU supports, and if the number of leaves reaches more than 0x80000001.
    // If so, then we can continue checking for NX.
    // Without this query before actually checking for NX, cpu_info_out[4] will store garbage, leading the kernel to
    // assume that NX exists at the worst case scenario, which can come back and bite us later with a nasty triple fault.
    // So please please please, if you are maintaining this (and if I really have maintainers at all), please keep this
    // code, one way or another.
    cpuid(CPUID_FUNC_X_MAXLEAF, cpu_info_out); // Number of leaves will be stored at EAX
    if (cpu_info_out[0] < CPUID_FUNC_X_GETFEATURES) return; // NX will certainly not be available to even check if it's available, let alone enabling.
    
    // Check for NX
    cpuid(CPUID_FUNC_X_GETFEATURES, cpu_info_out);
    mmu_data.cap.nx = (cpu_info_out[3] >> 20) & 1;
}

void mmu_init_paging(paddr_t page_dir_addr)
{
    if (mmu_data.present.pae) 
    {
        /*TODO*/
        return;
    }
    mmu_non_pae_init(page_dir_addr);
}

void mmu_reload_paging_addr()
{
    if (mmu_data.present.pae) 
    {
        /*TODO*/
        return;
    }
    mmu_non_pae_load_page_dir();
}