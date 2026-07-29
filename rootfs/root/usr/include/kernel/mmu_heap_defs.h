#pragma once
#include <stdint.h>
#include <bitmap.h>
#include <stdbool.h>

#define MAX_SIZE_CLASS_CNT 9
#define MIN_HEAP_ORDER 0
#define MAX_HEAP_ORDER 8
#define MAX_HEAP_SIZE (64 * 1024 * 1024)
#define HEAP_MAGIC_MARK 0xBADD1E6F
#define VMM_MAGIC_MARK 0xFEEB1ED1

typedef struct mmu_heap_node_t mmu_heap_node_t;

typedef enum
{
    MMU_HEAP_PARTIAL,
    MMU_HEAP_USED,
    MMU_HEAP_FREE,
} mmu_heap_status_t;

typedef struct mmu_heap_node_t
{
    u32 magic;
    mmu_heap_node_t* prev;
    mmu_heap_node_t* next;
    u32 block_size;
    u32 heap_size;
    u32 avail_cnt;
    union
    {
        struct
        {
            void* address;
        } vmm_region;
        struct
        {
            int list_type;
            u64* cur_free_qword;
            bitmap_t bmp;
        } heap;
    } impl; // Implementation-specific
} mmu_heap_node_t;

typedef mmu_heap_node_t* mmu_heap_list_t[MAX_SIZE_CLASS_CNT];

extern struct mmu_heap_shared_data_t
{
    void* start_range;
    mmu_heap_node_t* free_head;
    mmu_heap_node_t* vmm_region_head;
    mmu_heap_list_t used_head;
    mmu_heap_list_t partial_head;
} mmu_heap_shared_data;