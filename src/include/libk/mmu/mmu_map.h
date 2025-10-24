#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

#define MAX_REGION_COUNT 64

typedef enum
{
    MEMORY_TYPE_FREE = 1,
    MEMORY_TYPE_RESERVED,
    MEMORY_TYPE_ACPI,
    MEMORY_TYPE_ACPI_NVS,
    MEMORY_TYPE_BAD,
    MEMORY_TYPE_SYS_RESERVED,
    MEMORY_TYPE_HOLE,
    MEMORY_TYPE_UNIDENTIFIED,
} mmu_map_memtype_t;

typedef struct
{
    u32 entries_count;
    struct compatible_memregion_t{
        u64 base, length;
        u32 type;
        u32 acpi;
    } __attribute__((packed)) regions[MAX_REGION_COUNT];
} __attribute__((packed)) memory_info_t;

typedef struct memory_region_node_t memory_region_node_t;

typedef bool (*mmu_region_criterion_t)(memory_region_node_t* region);


void mmu_map_init(memory_info_t* info, void* pool);
void mmu_map_traverse_region();
void* mmu_map_allocate_region(u64 length);
void mmu_map_free_region(void* ptr);
