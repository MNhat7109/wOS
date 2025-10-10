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
    u64 base, length;
    u32 type;
    u32 acpi;
} __attribute__((packed)) memory_region_t;

typedef struct memory_region_node_t
{
    memory_region_t region;
    memory_region_node_t *prev, *next;
} __attribute__((packed)) memory_region_node_t;

typedef struct
{
    u32 entries_count;
    memory_region_t regions[MAX_REGION_COUNT];
} __attribute__((packed)) memory_info_t;

typedef bool (*mmu_region_criterion_t)(memory_region_node_t* region);

void mmu_map_init(memory_info_t* info);
int mmu_map_find_region(u64 base, memory_region_node_t* region_out);
void mmu_map_traverse_region();

memory_region_node_t* mmu_map_create_node(
    u64 base, 
    u64 length, 
    u32 type,
    u32 acpi
);
void mmu_map_erase_node(memory_region_node_t* node_addr);

memory_region_node_t* mmu_map_insert_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi,
    mmu_region_criterion_t can_insert
);
memory_region_node_t* mmu_map_delete_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi,
    mmu_region_criterion_t can_delete
);

memory_region_node_t* mmu_map_split_region
(    
    memory_region_node_t* node, 
    u64 base 
);
void mmu_map_merge_region_forward(    
    memory_region_node_t* node
);
void mmu_map_merge_region_backward(    
    memory_region_node_t* node
);