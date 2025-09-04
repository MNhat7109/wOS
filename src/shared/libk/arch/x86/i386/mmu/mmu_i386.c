#include <libk/mmu/mmu.h>
#include <libk/mmu/page_allocator.h>

#include <libk/bitmap/bitmap.h>
#include <libk/stdint.h>
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)

#include <arch/x86/utils/boot.h>

#endif

#ifdef __i386__

#include <arch/x86/i386/paging.h>

static struct
{
    bool status;
} mmu_i386_data;

void mmu_enable_paging(void* addr, bool use_pae)
{
    i386_load_paging((usize)addr);
    if (use_pae) i386_enable_pae();

    i386_enable_paging();

    mmu_i386_data.status = true;
}

bool mmu_get_status()
{
    return mmu_i386_data.status;
}

#endif