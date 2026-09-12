#include <kernel/mmio.h>
#include <kernel/mmu_vmem.h>

int mmio_setup_addr(void* addr, usize size)
{
    addr = mmu_vmem_alloc(addr, size, MMU_VMA_MMIO | MMU_VMA_FIXED | MMU_VMA_R | MMU_VMA_W | MMU_VMA_UC, addr);
    if (addr == ADDR_INVAL) return -1;
    return 0;
}

void mmio_outl(volatile u32* base, u32 offset, u32 value)
{
    if (!base) return;
    base[offset] = value;
}

void mmio_outq(volatile u64* base, u32 offset, u64 value)
{
    if (!base) return;
    base[offset] = value;
}

u32 mmio_inl(volatile u32* base, u32 offset)
{
    if (!base) return 0;
    return base[offset];
}

u64 mmio_inq(volatile u64* base, u32 offset)
{
    if (!base) return 0;
    return base[offset];
}