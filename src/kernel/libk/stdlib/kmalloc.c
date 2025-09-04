#include <libk/stdlib.h>
#include <libk/string.h>

#include <kutils/mmu/heap.h>

void* kmalloc(usize size)
{
    return mmu_allocate(size);
}

void* kcalloc(usize size)
{
    void* ptr = mmu_allocate(size);
    if (!ptr) return NULL;
    memset(ptr, 0, size);
    return ptr;
}

void* krealloc(void* ptr, usize size)
{
    return mmu_reallocate(ptr, size);
}

void kfree(void* ptr)
{
    mmu_free(ptr);
}