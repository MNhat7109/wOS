#include <kernel/mmu_frame.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu.h>
#include <kernel/debug.h>

#include <bitmap.h>

static struct
{
    bitmap_t frame_bmp;
} mmu_frame_data;

void mmu_frame_init(u8* bmp_buffer_addr, u64 mem_size)
{
    kdebugf(DEBUG_INFO, "MMU", "Bitmap buffer addr: 0x%x\n", bmp_buffer_addr);

    // Get total memory page count
    usize total_pages = mmu_byte_to_4k_pages(mem_size);

    // Initialize bitmap
    bitmap_init(&mmu_frame_data.frame_bmp, bmp_buffer_addr, total_pages);

    // Lock bitmap pages on the bitmap
    usize bmp_size = mmu_frame_data.frame_bmp.size;
    usize page_count = mmu_byte_to_4k_pages(bmp_size);

    kdebugf(DEBUG_INFO, "MMU", "Bitmap size: %u\n", bmp_size);
    mmu_frame_set_n((uptr)mmu_vtop((vaddr_t)bmp_buffer_addr), page_count);
}

void mmu_frame_set(uptr address)
{
    bitmap_set(&mmu_frame_data.frame_bmp, address>>12);
    mmu_inc_zone_size(MMU_ZONE_OS_RESERVED, 4096);
    mmu_dec_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_reserve(uptr address)
{
    bitmap_set(&mmu_frame_data.frame_bmp, address>>12);
    mmu_inc_zone_size(MMU_ZONE_HW_RESERVED, 4096);
    mmu_dec_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_clear(uptr address)
{
    bitmap_clear(&mmu_frame_data.frame_bmp, address>>12);
    mmu_dec_zone_size(MMU_ZONE_OS_RESERVED, 4096);
    mmu_inc_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_set_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
    {
        mmu_frame_set(address+i*4096);
    }
}

void mmu_frame_reserve_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
    {
        mmu_frame_reserve(address+i*4096);
    }
}

void mmu_frame_clear_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
    {
        mmu_frame_clear(address+i*4096);
    }
}

uptr mmu_frame_next()
{
    for (int i=0;i<mmu_frame_data.frame_bmp.bit_count; i++)
    {
        if (bitmap_get(&mmu_frame_data.frame_bmp, i) == 0)
        {
            uptr address = (uptr)(i<<12);
            mmu_frame_set(address);
            return address;
        }
    }

    return 0;
}