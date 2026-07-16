#include <kernel/mmu.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu_paging.h>
#include <kernel/debug.h>

typedef struct mmu_vma_range_t
{
    vaddr_t start_vaddr;
    vaddr_t end_vaddr;
    int flags;
} mmu_vma_range_t;

typedef struct mmu_vma_ptr_t mmu_vma_ptr_t;
typedef struct mmu_vma_ptr_t
{
    mmu_vma_range_t range;
    mmu_vma_ptr_t* next;
} mmu_vma_ptr_t;

static struct
{
    mmu_vma_ptr_t* head;
    int range_count;
    mmu_vma_ptr_t pool[128];
} mmu_vma_data;

int mmu_vmem_interpret_flags(int flags);
usize mmu_vmem_get_page_size(int flags);

vaddr_t mmu_vmem_find_space(vaddr_t min_range, usize length, u8 strict, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur);

mmu_vma_ptr_t* mmu_vmem_spawn(vaddr_t vaddr_start, vaddr_t vaddr_end, int flags);
void mmu_vmem_despawn(mmu_vma_ptr_t* ptr);

void mmu_vmem_init(){}

void* mmu_vmem_alloc(void* addr, usize len, int flags, void* args)
{
    addr = (void*)mmu_align_down((vaddr_t)addr, PAGE_SIZE);
    len = mmu_align_up(len, PAGE_SIZE);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Creating VMA range: va=0x%x, len=0x%x...\n", addr, len);
    
    mmu_vma_ptr_t* prev = NULL, *cur = NULL;
    vaddr_t va = mmu_vmem_find_space((vaddr_t)addr, len, (flags&MMU_VMA_FIXED), &prev, &cur);
    
    mmu_vma_ptr_t* new_vma = mmu_vmem_spawn(va, va+len, flags);
    if (!new_vma) return NULL;

    if (!prev)
        mmu_vma_data.head = new_vma;
    else
        (prev)->next = new_vma;
    new_vma->next = cur;

    // Parse flags for future operations
    
    usize pc = mmu_byte_to_4k_pages(len);
    paddr_t pa = 0;
    if (flags & MMU_VMA_ANON)
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Range is used for anonymous memory. Allocating from the PMM...\n");
        // PMM!!!!!!!
        pa = mmu_frame_alloc(pc);
        if (!pa) return NULL;
    }
    else if ((flags & MMU_VMA_MMIO) || (flags & MMU_VMA_PHYS))
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "Range is used for explicit backing or MMIO.\n");
        pa = (paddr_t)args;
    }
    
    // Map to page table
    kdebugf(DEBUG_INFO, MODULE_MMU, "Mapping range...\n");
    int page_attr = mmu_vmem_interpret_flags(flags);
    for (usize i=0;i<pc;i++) 
    {
        mmu_map_page(va+i*PAGE_SIZE, pa+i*PAGE_SIZE, page_attr);
    }

    return (void*)va;
}

void mmu_vmem_free(void* addr)
{
    vaddr_t vaddr_start = mmu_align_down((vaddr_t)addr, PAGE_SIZE);
    mmu_vma_ptr_t* found = NULL, *prev= NULL;
    for (mmu_vma_ptr_t* ptr = mmu_vma_data.head; ptr != NULL; ptr=ptr->next)
    {
        if (vaddr_start >= ptr->range.start_vaddr && vaddr_start < ptr->range.end_vaddr)
        {
            found = ptr;
            break;
        }
        prev = found;
    }
    if (!found)
    {
        return;
    }

    // Unlink whole block
    if (!prev) mmu_vma_data.head = found->next;
    prev->next = found->next;
    
    // Unmap behavior according to flags
    if (found->range.flags & MMU_VMA_ANON)
    {
        // Retrieve physical block from start range of VMA, free it.
        paddr_t pa = mmu_walk_page_table(found->range.start_vaddr);
        if (!pa) return;
        mmu_frame_free(pa);
    }

    // Unmap block
    for (vaddr_t va = found->range.start_vaddr; va < found->range.end_vaddr; va+=PAGE_SIZE)
    {
        int status = mmu_unmap_page(va);
    }

    mmu_vmem_despawn(found);
}

vaddr_t mmu_vmem_find_space(vaddr_t min_range, usize length, u8 strict, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur)
{
    *cur = mmu_vma_data.head;

    for (; *cur != NULL; *cur = (*cur)->next)
    {
        uptr base = *prev?(*prev)->range.end_vaddr:0; 
        u8 ok = strict?base==min_range:base>=min_range;
        if (ok && base + length < (*cur)->range.start_vaddr)
        {
            return base;
        }
        *prev = *cur;
    }

    return min_range;
}

mmu_vma_ptr_t* mmu_vmem_spawn(vaddr_t vaddr_start, vaddr_t vaddr_end, int flags)
{
    // TODO: Integrate heap later when heap is set up

    if (mmu_vma_data.range_count>=128)
    {
        // TODO: Maybe we need a panic function?
        return NULL;
    }

    mmu_vma_data.pool[mmu_vma_data.range_count] = (mmu_vma_ptr_t){
        .range = {
            .start_vaddr = vaddr_start,
            .end_vaddr = vaddr_end,
            .flags = flags
        }
    };
    return &mmu_vma_data.pool[mmu_vma_data.range_count++];
}

void mmu_vmem_despawn(mmu_vma_ptr_t* ptr)
{

}