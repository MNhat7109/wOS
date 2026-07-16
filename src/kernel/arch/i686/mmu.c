#include <kernel/mmu.h>
#include <kernel/arch/i686/mmu.h>
#include <kernel/mmu_frame.h>
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
usize HUGE_PAGE_SIZE(){ return (mmu_data.present.pae?HUGE_PAGE_SIZE_PAE:HUGE_PAGE_SIZE_NO_PAE); }

bool full_paging_on=false;

void mmu_check_features();
void mmu_init_paging();
void mmu_reload_paging_addr();

void __attribute__((cdecl)) mmu_load_address_space_i686(paddr_t paddr);
void __attribute__((cdecl)) mmu_reload_address_space_i686();
void __attribute__((cdecl)) mmu_enable_paging();
void __attribute__((cdecl)) mmu_enable_pse();

void mmu_non_pae_init(paddr_t page_dir_addr);
void mmu_non_pae_load_page_dir();

int mmu_map_page_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
int mmu_map_page_huge_non_pae(vaddr_t vaddr, paddr_t paddr, u32 attributes);
int mmu_unmap_page_non_pae(vaddr_t vaddr);

int mmu_map_page_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
int mmu_map_page_huge_pae(vaddr_t vaddr, paddr_t paddr, u64 attributes);
int mmu_unmap_page_pae(vaddr_t vaddr);

paddr_t mmu_walk_page_table_non_pae(vaddr_t vaddr);
paddr_t mmu_walk_page_table_pae(vaddr_t vaddr);

void mmu_arch_init(u32 optional_features)
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
    mmu_init_paging();
}

void mmu_load_address_space(paddr_t paddr)
{
    mmu_load_address_space_i686(paddr);
}

void mmu_reload_address_space()
{
    mmu_reload_address_space();
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
    full_paging_on = true;
}

paddr_t mmu_vtop(vaddr_t vaddr)
{
    return (paddr_t)(vaddr-HIGHER_HALF_OFFSET);
}    

vaddr_t mmu_ptov(paddr_t paddr)
{
    return (vaddr_t)(paddr+HIGHER_HALF_OFFSET);
}    

int mmu_map_page(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    if (mmu_data.present.pae)
    {
        if (!mmu_data.present.nx)
            attributes &= ~MMU_PG_ATTR_NX;
        // mmu_map_page_pae(vaddr, paddr, attributes);
        return 0;
    }
    return mmu_map_page_non_pae(vaddr, paddr,attributes);
}

int mmu_map_page_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    if (!mmu_data.present.pse) return -1;

    if (mmu_data.present.pae)
    {
        if (!mmu_data.present.nx)
            attributes &= ~MMU_PG_ATTR_NX;
        // mmu_map_page_huge_pae(vaddr, paddr, attributes);
        return 0;
    }
    return mmu_map_page_huge_non_pae(vaddr, paddr,attributes);
}

int mmu_map_page_very_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes)
{
    // Stub only in i386, will do nothing
    return 0;
}

int mmu_unmap_page(vaddr_t vaddr)
{
    if (mmu_data.present.pae)
    {
        // return mmu_unmap_page_pae(vaddr, paddr_out);
    }
    return mmu_unmap_page_non_pae(vaddr);
}

paddr_t mmu_walk_page_table(vaddr_t vaddr)
{
    if (mmu_data.present.pae)
    {
        // return mmu_walk_page_table_pae(vaddr);
    }
    return mmu_walk_page_table_non_pae(vaddr);
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

void mmu_init_paging()
{
    uptr first_free_page = mmu_frame_alloc(1);
    if (mmu_data.present.pae) 
    {
        /*TODO*/
        return;
    }
    mmu_non_pae_init(first_free_page);
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