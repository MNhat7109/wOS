#include <libk/mmu/mmu.h>
#include <libk/mmu/page_allocator.h>

#include <libk/bitmap/bitmap.h>
#include <libk/stdint.h>
#include <libk/string.h>
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)

#include <arch/x86/utils/boot.h>

#endif

#ifdef __i386__

#include <arch/x86/i386/paging.h>

static struct
{
    void* bmp_buffer;
    bool status, pae;
    void* paging_addr;
} mmu_i386_data;


void mmu_init(boot_info_t* info, void* buffer, bool use_pae)
{    
    mmu_i386_data.pae = use_pae;
    mmu_init_map((memory_info_t*)info->mem_map);

    mmu_i386_data.bmp_buffer = buffer; 
    page_alloc_init(mmu_i386_data.bmp_buffer);

    mmu_i386_data.paging_addr = page_alloc_request();
    memset(mmu_i386_data.paging_addr, 0, 0x1000);
    mmu_init_pages(mmu_i386_data.paging_addr, mmu_i386_data.pae);
}

void mmu_enable()
{
    i386_load_paging((usize)mmu_i386_data.paging_addr);
    if (mmu_i386_data.pae) i386_enable_pae();
    i386_enable_paging();

    mmu_i386_data.status = true;
}

bool mmu_get_status()
{
    return mmu_i386_data.status;
}

#endif