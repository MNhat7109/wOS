#pragma once

#define MEMORY_TYPE_FREE 1
#define MEMORY_TYPE_RESERVED 2
#define MEMORY_TYPE_ACPI 3
#define MEMORY_TYPE_ACPI_NVS 4
#define MEMORY_TYPE_BAD 5

typedef struct
{
    u64 base, length;
    u32 type;
    u32 acpi;
} __attribute__((packed)) memory_region_t;

typedef struct
{
    u32 entries_count;
    memory_region_t regions[];
} __attribute__((packed)) memory_info_t;

void mmu_init(memory_info_t* mem_map, u64 total_mem_size, uptr first_free_addr);

