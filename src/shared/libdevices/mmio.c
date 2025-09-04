#include <devices/mmio.h>

#include <libk/mmu/mmu.h>

static u32 mmio_read_32(struct mmio_info_t* self, u32 offset);
static u64 mmio_read_64(struct mmio_info_t* self, u32 offset);
static void mmio_write_32(struct mmio_info_t* self, u32 offset, u32 value);
static void mmio_write_64(struct mmio_info_t* self, u32 offset, u64 value);

mmio_layer_t mmio_layer = {
    .readl = &mmio_read_32,
    .readq = &mmio_read_64,
    .writel = &mmio_write_32,
    .writeq = &mmio_write_64
};

void mmio_acquire(
    struct mmio_info_t* self, 
    u64 base, 
    u64 size, 
    u32 flags
)
{
    self->base = base;
    self->size = size;
    self->flags = flags;
    self->layer = &mmio_layer;

    // Process the flags
    u64 mmu_flags = MMU_PT_FLAG_READ_WRITE;
    if (self->flags & MMIO_FLAG_UNCACHEABLE)
        mmu_flags |= MMU_PT_FLAG_CACHE_DISABLE;
    if (self->flags & MMIO_FLAG_WRITE_THRU)
        mmu_flags |= MMU_PT_FLAG_WRITE_THRU;
    
    // Identity map the MMIO base
    usize page_count = mmu_byte_to_page_count(self->size);
    mmu_mmapn(self->base, 0, page_count, mmu_flags);
}

void mmio_release(struct mmio_info_t* self)
{
    u64 page_count = mmu_byte_to_page_count(self->size);
    mmu_munmapn(self->base, &page_count);

    self->layer=NULL;
    self->base =0;
    self->size =0;
    self->flags=0;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

static u32 mmio_read_32(struct mmio_info_t* self, u32 offset)
{
    if (offset >= self->size) return 0;
    return *((volatile u32*)(self->base+offset));
}

static u64 mmio_read_64(struct mmio_info_t* self, u32 offset)
{
    if (offset >= self->size) return 0;
    return *((volatile u64*)(self->base+offset));
}

static void mmio_write_32(struct mmio_info_t* self, u32 offset, u32 value)
{
    if (offset >= self->size) return;
    *((volatile u32*)(self->base+offset)) = value;
}

static void mmio_write_64(struct mmio_info_t* self, u32 offset, u64 value)
{
    if (offset >= self->size) return;
    *((volatile u64*)(self->base+offset)) = value;
}