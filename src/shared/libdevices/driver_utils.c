#include <devices/driver.h>

#include <libk/mmu/page_allocator.h>
#include <libk/mmu/mmu.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <libk/stdatomic.h>
#include <stdarg.h>

#define DRIVER_ALLOC_MAGIC 0xD1EDB117

typedef struct driver_obj_hdr_t
{
    u32 magic;
    generic_driver_alloc_flags_t alloc_flags;
    usize size;
} driver_obj_hdr_t;

void* driver_alloc(
    struct generic_driver_tree_node_t* driver, 
    usize size, 
    generic_driver_alloc_flags_t alloc_flags,
    usize mmu_specific
)
{
    if (size == 0) return NULL;

    driver_obj_hdr_t* obj;
    usize block_size = size+sizeof(driver_obj_hdr_t);

    switch (alloc_flags)
    {
        case DRIVER_ALLOC_FLAG_HEAP:
            obj = kmalloc(block_size);
            break;
        case DRIVER_ALLOC_FLAG_ID:
            usize block_page_count = mmu_byte_to_page_count(block_size);
            obj = page_alloc_request();
            mmu_mmap(obj, obj, mmu_specific);
            for (usize i=0;i<block_page_count-1;i++)
            {
                void* addr = page_alloc_request();
                mmu_mmap(addr, addr, mmu_specific);
            }
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown allocation flag\n");
            break;
    }

    // Populate data
    if (!obj) return NULL;
    
    obj->magic = DRIVER_ALLOC_MAGIC;
    obj->alloc_flags = alloc_flags;
    obj->size = size;

    return (void*)((u8*)obj+sizeof(driver_obj_hdr_t));
}

void driver_free(
    struct generic_driver_tree_node_t* driver, 
    void* ptr
)
{
    if (!ptr) return;

    driver_obj_hdr_t* hdr = 
    (driver_obj_hdr_t*)((u8*)ptr-sizeof(driver_obj_hdr_t));
    if (hdr->magic != DRIVER_ALLOC_MAGIC)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Invalid header structure\n");
        return;
    }

    switch (hdr->alloc_flags)
    {
        case DRIVER_ALLOC_FLAG_HEAP:
            kfree(hdr);
            break;
        case DRIVER_ALLOC_FLAG_ID:
            usize block_page_count = mmu_byte_to_page_count(hdr->size);
            usize base_start = mmu_munmapn(hdr, block_page_count);
            page_alloc_freen((void*)base_start, block_page_count);
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown allocation flag\n");
            break;
    }
}

void driver_ref(struct generic_driver_tree_node_t* driver)
{
    // Increase the driver's refcount
    ATOMIC_ADD(driver->refcount, 1);
}

void driver_deref(struct generic_driver_tree_node_t* driver)
{
    if (ATOMIC_LOAD(driver->refcount) == 1) return;
    // Decrease the driver's refcount
    ATOMIC_SUB(driver->refcount, 1);
}