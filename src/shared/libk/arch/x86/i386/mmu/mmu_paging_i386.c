#include <libk/stdint.h>
#include <libk/mmu/page_allocator.h>
#include <libk/mmu/mmu.h>
#include <libk/mmu/mmu_paging.h>
#include <libk/string.h>

#ifdef __i386__

#include <arch/x86/i386/paging.h>
#include <stdbool.h>

static struct
{
    union
    {
        struct
        {
            u8 paging : 1;
            u8 pae : 1;
            u32 _reserved : 30;
        } __attribute__((packed)) fields;
        u32 bits;
    } features;
    u64* page_dir_ptr;
    u32* page_dir;
} mmu_i386_paging;

void mmu_paging_init(void* addr)
{
    // Check for MMU support
    // TODO
    mmu_i386_paging.features.fields.pae = false;

    if (mmu_i386_paging.features.fields.pae) 
    mmu_i386_paging.page_dir_ptr = (u64*)addr;
    else 
    mmu_i386_paging.page_dir = (u32*)addr;
}

void mmu_paging_enable()
{
    if (mmu_i386_paging.features.fields.pae)
    {
        i386_load_paging((usize)mmu_i386_paging.page_dir_ptr);
        i386_enable_pae();
    }
    else
    {
        i386_load_paging((usize)mmu_i386_paging.page_dir);
    }
    i386_enable_paging();
    mmu_i386_paging.features.fields.paging = true;
}

u32 mmu_paging_get_features()
{
    return mmu_i386_paging.features.bits;
}

void* mmu_paging_get_addr()
{
    if (mmu_i386_paging.features.fields.pae)
        return mmu_i386_paging.page_dir_ptr;
    return mmu_i386_paging.page_dir;
}

#endif