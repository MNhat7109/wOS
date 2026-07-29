#include <kernel/mmu_vmem.h>
#include <kernel/mmu_heap_defs.h>
#include <kernel/debug.h>

#define STATUS_INCOMPATIBLE -1
#define STATUS_NORMAL_ERROR 0

#define FIND_HEAP_HDR_VMM (mmu_heap_node_t*)0x67

void mmu_heap_unregister_vmm_region(mmu_heap_node_t* node);
void mmu_heap_update_chunk_list(mmu_heap_node_t* node);


mmu_heap_node_t* mmu_heap_find_vmm_header(void* ptr)
{
    // Traverse the linked list for header
    for (mmu_heap_node_t* vmm_hdr = mmu_heap_shared_data.vmm_region_head; vmm_hdr; vmm_hdr = vmm_hdr->next)
    {
        if (ptr >= vmm_hdr->impl.vmm_region.address && ptr < vmm_hdr->impl.vmm_region.address+vmm_hdr->heap_size)
        {
            return vmm_hdr;
        }
    }

    kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Tried to free a stray pointer\n");
    return NULL;

}

mmu_heap_node_t* mmu_heap_find_heap_header(void* ptr)
{
    // Align pointer down to 4K boundary
        // The reason for this lies in the structure of the heap.
    // As the heap is comprised of many heap chunks, each chunk has a structure of:
    // + - - - - + - - - - - - - - - - + - - - - - - + 
    // |  Header | Bitmap rep. of data |    Data     |
    // + - - - - + - - - - - - - - - - + - - - - - - +
    // <---------- 4KiB --------------><--- 4KiB ---->
    //                                      ^
    //                                      |___(Pointer lies here)
    // In order to find the real header from a pointer address, we trace it back to the header area, which lies at the start of the 8KB chunk, hence the 8K alignment
    // Next, check for the magic mark. If exists, yay, that's our header right there.
    // Otherwise, there will be 2 cases:
    //      1. The pointer belongs to the registered VMM region. In order for this to be true, we will have to traverse the linked list to find out if the address lies between [start_address, start_address+size), where start_address <= ptr
    //      2. The pointer is not acknowledged by the heap itself (a stray pointer) In this case, return directly as the pointer is now out of reach

    // Check for region validity first.
    mmu_heap_node_t* vmm_hdr = mmu_heap_find_vmm_header(ptr);
    if (!vmm_hdr) 
    {
        return NULL;
    }
    
    mmu_heap_node_t* hdr = (mmu_heap_node_t*)mmu_align_down((uptr)ptr-0x1000, (PAGE_SIZE));

    if (hdr->magic==HEAP_MAGIC_MARK)
    {
        kdebugf(DEBUG_INFO, MODULE_MMU, "This is a heap region\n");
        return hdr;
    }
    
    kdebugf(DEBUG_WARN, MODULE_MMU, "This is a VMM region\n");
    return FIND_HEAP_HDR_VMM;
}

int mmu_heap_free_vmem(void* ptr)
{
    mmu_heap_node_t* node = mmu_heap_find_vmm_header(ptr);
    if (!node) return STATUS_NORMAL_ERROR;

    mmu_vmem_free(node->impl.vmm_region.address);

    // Unregister VMM
    mmu_heap_unregister_vmm_region(node);
    return 1;

}

int mmu_heap_free_regular(void* ptr)
{
    mmu_heap_node_t* node = mmu_heap_find_heap_header(ptr);
    if (!node)
    {
        return STATUS_NORMAL_ERROR;
    }

    if (node == FIND_HEAP_HDR_VMM)
    {
        return STATUS_INCOMPATIBLE;
    }

    // Find the bit position where the pointer resides
    u8* user_data = (u8*)node+PAGE_SIZE;
    usize bit_pos = ((u8*)ptr-user_data)/node->block_size;

    // Clear the bit if set, freak out otherwise.
    if (bitmap_get(&node->impl.heap.bmp, bit_pos) == 0)
    {
        // error!
        kdebugf(DEBUG_CRITICAL, MODULE_MMU, "Double free detected\n");
        return STATUS_NORMAL_ERROR;
    }

    bitmap_clear(&node->impl.heap.bmp, bit_pos);

    // To find what QWORD the bit position resides, take bit_pos / 64 bit
    u64* freed_qword = ((u64*)node->impl.heap.bmp.buffer)+(bit_pos>>6);

    // Update cur_free_qword to the QWORD holding the freed bit, only if cur_free_qword is NULL (the chunk was previously filled)
    if (!node->impl.heap.cur_free_qword)
    {
        node->impl.heap.cur_free_qword = freed_qword;
    }
    node->avail_cnt++;

    // Update the chunk status (Set status, move chunk to another list etc.)
    mmu_heap_update_chunk_list(node);
    return 1;
}

void mmu_heap_free(void* ptr)
{
    // Find the header in which the pointer resides
    kdebugf(DEBUG_INFO, MODULE_MMU, "Freeing pointer at addr 0x%p...\n", ptr);

    int status = mmu_heap_free_regular(ptr);
    if (status == STATUS_INCOMPATIBLE)
    mmu_heap_free_vmem(ptr);
}