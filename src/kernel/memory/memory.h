#pragma once
#include "../stdint.h"

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

extern memory_info_t* mem_map;

void memory_init(memory_info_t* mem_info);
void memory_view_map();
void memory_sort_regions();
void memory_create_region(u64 base, u64 length, u32 type);
void memory_merge_region();

u32 memory_get_total_size_bytes();

void memory_init_alloc(u32 address, u32 page_count);
void* memory_allocate(u32 size);
void memory_free(void* block);
void memory_destroy_alloc();