#include <kernel/mmu_frame.h>
#include <kernel/mmu_frame_bmp.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu.h>
#include <kernel/debug.h>

#include <bitmap.h>

static struct
{
    bitmap_t frame_bmp;
} mmu_frame_bmp_data;

void mmu_frame_bmp_init(mmu_frame_allocator_t* m_alloc, u8* bmp_buffer_addr, u64 mem_size);
uptr mmu_frame_bmp_alloc(mmu_frame_allocator_t* m_alloc, u64 block_size);
void mmu_frame_bmp_free(mmu_frame_allocator_t* m_alloc, uptr address);

void mmu_frame_bmp_set(uptr address);
void mmu_frame_bmp_reserve(uptr address);
void mmu_frame_bmp_clear(uptr address);

void mmu_frame_bmp_set_n(uptr address, usize n);
void mmu_frame_bmp_reserve_n(uptr address, usize n);
void mmu_frame_bmp_clear_n(uptr address, usize n);
uptr mmu_frame_bmp_next(u64 size);

mmu_frame_bmp_allocator_ops_t alloc_bmp = {
    .hdr = {
        .alloc = &mmu_frame_bmp_alloc,
        .free = &mmu_frame_bmp_free,
        .init = &mmu_frame_bmp_init,
    },
    .free_pages = &mmu_frame_bmp_clear_n,
    .lock_pages = &mmu_frame_bmp_set_n,
    .reserve_pages = &mmu_frame_bmp_reserve_n,
};

void mmu_frame_bmp_init(mmu_frame_allocator_t* m_alloc, u8* bmp_buffer_addr, u64 mem_size)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap buffer addr: 0x%x\n", bmp_buffer_addr);

    // Get total memory page count
    usize total_pages = mmu_byte_to_4k_pages(mem_size);

    // Initialize bitmap
    bitmap_init(&mmu_frame_bmp_data.frame_bmp, bmp_buffer_addr, total_pages);

    // Lock bitmap pages on the bitmap
    usize bmp_size = mmu_frame_bmp_data.frame_bmp.size;
    usize page_count = mmu_byte_to_4k_pages(bmp_size);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Bitmap size: %u\n", bmp_size);
    mmu_frame_bmp_set_n((uptr)mmu_vtop((vaddr_t)bmp_buffer_addr), page_count);

    m_alloc->mem_state = &mmu_frame_bmp_data.frame_bmp;
}

uptr mmu_frame_bmp_alloc(mmu_frame_allocator_t* m_alloc, u64 block_size)
{
	usize page_count = mmu_byte_to_4k_pages(block_size);
	for (int i=0;i<mmu_frame_bmp_data.frame_bmp.bit_count;i++)
	{
		if (bitmap_get(&mmu_frame_bmp_data.frame_bmp, i) == 0)
		{
			uptr addr = i << 12;
			mmu_frame_bmp_set_n(addr, page_count);
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

void mmu_frame_bmp_set_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
    {
        mmu_frame_bmp_set(address+i*4096);
    }
}

void mmu_frame_bmp_reserve_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
    {
        mmu_frame_bmp_reserve(address+i*4096);
    }
}

void mmu_frame_bmp_clear_n(uptr address, usize n)
{
    for (usize i=0;i<n;i++)
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

const mmu_frame_allocator_ops_t* mmu_frame_bmp_load_ops()
{
    return (mmu_frame_allocator_ops_t*)&alloc_bmp;
}
