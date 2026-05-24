#include <kernel/mmu_frame.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu.h>
#include <kernel/debug.h>

#include <bitmap.h>

static struct
{
    bitmap_t frame_bmp;
} mmu_frame_bmp_data;

int mmu_frame_bmp_init(mmu_frame_allocator_t* m_alloc, uptr bmp_buffer_addr, u64 mem_size);
uptr mmu_frame_bmp_alloc(mmu_frame_allocator_t* m_alloc, u64 page_count);
void mmu_frame_bmp_free(mmu_frame_allocator_t* m_alloc, uptr address);

void mmu_frame_bmp_set(uptr address);
void mmu_frame_bmp_reserve(uptr address);
void mmu_frame_bmp_clear(uptr address);

void mmu_frame_bmp_set_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n);
void mmu_frame_bmp_reserve_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n);
void mmu_frame_bmp_clear_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n);
uptr mmu_frame_bmp_next(u64 size);

mmu_frame_plugins_t bmp_plugin = {
    .init = &mmu_frame_bmp_init,
    .ops = {
        .alloc = &mmu_frame_bmp_alloc,
        .free = &mmu_frame_bmp_free,
        .reserve_pages = &mmu_frame_bmp_reserve_n,
        .lock_pages = &mmu_frame_bmp_set_n,
        .release_pages = &mmu_frame_bmp_clear_n,
    }
};

int mmu_frame_bmp_init(mmu_frame_allocator_t* m_alloc, uptr bmp_buffer_addr, u64 mem_size)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap buffer addr: 0x%x\n", bmp_buffer_addr);

    // Get total memory page count
    u64 total_pages = mmu_byte_to_4k_pages(mem_size);

    // Initialize bitmap
    bitmap_init(&mmu_frame_bmp_data.frame_bmp, (u8*)bmp_buffer_addr, total_pages);

    m_alloc->meta_offset_vaddr = bmp_buffer_addr;
    m_alloc->meta_size = mmu_frame_bmp_data.frame_bmp.size;

    // Lock bitmap pages on the bitmap
    u64 page_count = mmu_byte_to_4k_pages(m_alloc->meta_size);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap size: %u\n", m_alloc->meta_size);
    mmu_frame_bmp_set_n(m_alloc,(uptr)mmu_vtop((vaddr_t)bmp_buffer_addr), page_count);

    return 0;
}

uptr mmu_frame_bmp_alloc(mmu_frame_allocator_t* m_alloc, u64 page_count)
{
	for (int i=0;i<mmu_frame_bmp_data.frame_bmp.bit_count;i++)
	{
		if (bitmap_get(&mmu_frame_bmp_data.frame_bmp, i) == 0)
		{
			uptr addr = i << 12;
			mmu_frame_bmp_set_n(m_alloc,addr, page_count);
			return addr;
		}
	}
	return 0;
}

void mmu_frame_bmp_free(mmu_frame_allocator_t* m_alloc, uptr address)
{
	
}

void mmu_frame_bmp_set(uptr address)
{
    bitmap_set(&mmu_frame_bmp_data.frame_bmp, address>>12);
    mmu_inc_zone_size(MMU_ZONE_OS_RESERVED, 4096);
    mmu_dec_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_bmp_reserve(uptr address)
{
    bitmap_set(&mmu_frame_bmp_data.frame_bmp, address>>12);
    mmu_inc_zone_size(MMU_ZONE_HW_RESERVED, 4096);
    mmu_dec_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_bmp_clear(uptr address)
{
    bitmap_clear(&mmu_frame_bmp_data.frame_bmp, address>>12);
    mmu_dec_zone_size(MMU_ZONE_OS_RESERVED, 4096);
    mmu_inc_zone_size(MMU_ZONE_FREE, 4096);
}

void mmu_frame_bmp_set_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "0x%x\n", address);
    for (u64 i=0;i<n;i++)
    {
        mmu_frame_bmp_set(address+i*4096);
    }
}

void mmu_frame_bmp_reserve_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n)
{
    for (u64 i=0;i<n;i++)
    {
        mmu_frame_bmp_reserve(address+i*4096);
    }
}

void mmu_frame_bmp_clear_n(mmu_frame_allocator_t* m_alloc, uptr address, u64 n)
{
    for (u64 i=0;i<n;i++)
    {
        mmu_frame_bmp_clear(address+i*4096);
    }
}

u32 mmu_frame_bmp_next_bit()
{
    for (u32 i=0;i<mmu_frame_bmp_data.frame_bmp.bit_count; i++)
    {
        if (bitmap_get(&mmu_frame_bmp_data.frame_bmp, i) == 0)
        {
            return i;
        }
    }

    return 0;
}

const bitmap_t* mmu_frame_bmp_get_metadata()
{
    return &mmu_frame_bmp_data.frame_bmp;
}

const mmu_frame_plugins_t* mmu_frame_bmp_get_plugin()
{
    return &bmp_plugin;
}