#include <kernel/mmu.h>
#include <kernel/mmu_other.h>
#include <kernel/mmu_frame.h>
#include <kernel/mmu_vmem.h>
#include <kernel/mmu_paging.h>
#include <kernel/debug.h>
#include <stdbool.h>

#define MMU_VMEM_MAX_VMA_ENTRY 4096
#define MMU_VMEM_ADDR_INVAL ADDR_INVAL

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
    vaddr_t tail_addr;
    int range_count;
    usize vma_node_region_size;
    mmu_vma_ptr_t pool[MMU_VMEM_MAX_VMA_ENTRY];
} mmu_vma_data;

int mmu_vmem_interpret_flags(int flags);
usize mmu_vmem_get_page_size(int flags);

vaddr_t mmu_vmem_find_space(vaddr_t min_range, usize length, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur);
vaddr_t mmu_vmem_check_space(vaddr_t start_va, usize length, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur);

mmu_vma_ptr_t* mmu_vmem_spawn(vaddr_t vaddr_start, vaddr_t vaddr_end, int flags);
void mmu_vmem_despawn(mmu_vma_ptr_t* ptr);

void mmu_vmem_init(uptr tail_addr)
{
    kdebugf(DEBUG_INFO, MODULE_MMU, "Starting up VMM...\n");
    mmu_vma_data.tail_addr = tail_addr;
    // TODO: For later, when there's an opportunity to reserve 4M for VMA region in linker script
    mmu_vma_data.vma_node_region_size = 4*1024*1024;
    kdebugf(DEBUG_INFO, MODULE_MMU, "VMM started up\n");
}

void* mmu_vmem_alloc(void* addr, usize len, int flags, void* args)
{
    addr = (void*)mmu_align_down((vaddr_t)addr, PAGE_SIZE);
    len = mmu_align_up(len, PAGE_SIZE);

    kdebugf(DEBUG_INFO, MODULE_MMU, "Creating VMA range: start_va=0x%x, len=0x%x...\n", addr, len);
    
    mmu_vma_ptr_t* prev = NULL, *cur = NULL;
    vaddr_t va = (flags&MMU_VMA_FIXED)?
    mmu_vmem_check_space((vaddr_t)addr, len, &prev, &cur):
    mmu_vmem_find_space((vaddr_t)addr, len, &prev, &cur);

    if (va == MMU_VMEM_ADDR_INVAL)
    {
        return (void*)MMU_VMEM_ADDR_INVAL;
    }

    kdebugf(DEBUG_INFO, MODULE_MMU, "Found VMA range: va=0x%x, len=0x%x.\n", va, len);
    
    mmu_vma_ptr_t* new_vma = mmu_vmem_spawn(va, va+len, flags);
    if (!new_vma) return (void*)MMU_VMEM_ADDR_INVAL;

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
        if (!pa) return (void*)MMU_VMEM_ADDR_INVAL;
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
    kdebugf(DEBUG_INFO, MODULE_MMU, "Finished\n");

    return (void*)va;
}

void mmu_vmem_free(void* addr)
{
    vaddr_t vaddr_start = mmu_align_down((vaddr_t)addr, PAGE_SIZE);
    kdebugf(DEBUG_INFO, MODULE_MMU, "Freeing VMA range: start_va=0x%x...\n", vaddr_start);
    mmu_vma_ptr_t* found = NULL, *prev= NULL;
    for (mmu_vma_ptr_t* ptr = mmu_vma_data.head; ptr != NULL; ptr=ptr->next)
    {
        if (vaddr_start >= ptr->range.start_vaddr && vaddr_start < ptr->range.end_vaddr)
        {
            kdebugf(DEBUG_INFO, MODULE_MMU, "Found range at start_va=0x%x, end_va=0x%x...\n", ptr->range.start_vaddr, ptr->range.end_vaddr);
            found = ptr;
            break;
        }
        prev = ptr;
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

vaddr_t mmu_vmem_find_space(vaddr_t min_range, usize length, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur)
{
    if (!mmu_vma_data.head) return min_range;

    *cur = mmu_vma_data.head;
    uptr desired = 0;

    for (; *cur != NULL; *cur = (*cur)->next)
    {
        desired = *prev?(*prev)->range.end_vaddr:0; 

        if (desired >= min_range && (*cur)->range.start_vaddr - desired >= length)
        {
            kdebugf(DEBUG_INFO, MODULE_MMU, "Found. base=0x%x\n", desired);
            break;
        }
        *prev = *cur;
    }

    desired = (*prev)->range.end_vaddr;
    if (mmu_vma_data.tail_addr - desired >= length) return desired;
    return MMU_VMEM_ADDR_INVAL;
}

vaddr_t mmu_vmem_check_space(vaddr_t start_va, usize length, mmu_vma_ptr_t** prev, mmu_vma_ptr_t** cur)
{
    if (!mmu_vma_data.head) return start_va;

    *cur = mmu_vma_data.head;

    for (; *cur != NULL; *cur = (*cur)->next)
    {
        uptr base = *prev?(*prev)->range.end_vaddr:0; 
        
        if (base > start_va) break;
        if (start_va + length <= (*cur)->range.start_vaddr)
        {
            kdebugf(DEBUG_INFO, MODULE_MMU, "Found. base=0x%x\n", start_va);
            return start_va;
        }

        *prev = *cur;
    }

    return MMU_VMEM_ADDR_INVAL;
}

mmu_vma_ptr_t* mmu_vmem_spawn(vaddr_t vaddr_start, vaddr_t vaddr_end, int flags)
{
    mmu_vma_ptr_t* ptr = NULL;

    if (mmu_vma_data.range_count>=MMU_VMEM_MAX_VMA_ENTRY)
    {
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Out of free VMAs\n");
        return NULL;
    }

    ptr = &mmu_vma_data.pool[mmu_vma_data.range_count++];

populate:
    *ptr = (mmu_vma_ptr_t){
        .range = {
            .start_vaddr = vaddr_start,
            .end_vaddr = vaddr_end,
            .flags = flags
        },
    };
    kdebugf(DEBUG_INFO, MODULE_MMU, "ptr=0x%x, start=0x%x, end=0x%x...\n", ptr, ptr->range.start_vaddr, ptr->range.end_vaddr);
    return ptr;
}

void mmu_vmem_despawn(mmu_vma_ptr_t* ptr)
{
}