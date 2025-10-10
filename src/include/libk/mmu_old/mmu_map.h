#pragma once
#include <libk/stdint.h>

#define MAX_REGION_COUNT 64

typedef enum
{
    MEMORY_TYPE_FREE = 1,
    MEMORY_TYPE_RESERVED,
    MEMORY_TYPE_ACPI,
    MEMORY_TYPE_ACPI_NVS,
    MEMORY_TYPE_BAD,
    MEMORY_TYPE_SYS_RESERVED,
} mmu_map_memtype_t;

typedef struct
{
    u64 base, length;
    u32 type;
    u32 acpi;
} __attribute__((packed)) memory_region_t;

typedef struct
{
    u32 entries_count;
    memory_region_t regions[MAX_REGION_COUNT];
} __attribute__((packed)) memory_info_t;

void mmu_map_init(memory_info_t* mem_info);
memory_info_t* mmu_map_get();
void mmu_map_show();
void mmu_map_sort();
void mmu_map_create_region(u64 base, u64 length, u32 type, memory_region_t* region_out);
void mmu_map_sanitize();