#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

#define MEMORY_TYPE_FREE 1
#define MEMORY_TYPE_RESERVED 2
#define MEMORY_TYPE_ACPI 3
#define MEMORY_TYPE_ACPI_NVS 4
#define MEMORY_TYPE_BAD 5

#define MMU_PT_FLAG_PRESENT       (1ULL<<0)
#define MMU_PT_FLAG_READ_WRITE    (1ULL<<1)
#define MMU_PT_FLAG_USER_SUPER    (1ULL<<2)
#define MMU_PT_FLAG_WRITE_THRU    (1ULL<<3)
#define MMU_PT_FLAG_CACHE_DISABLE (1ULL<<4)
#define MMU_PT_FLAG_ACCESSED      (1ULL<<5)
#define MMU_PT_FLAG_DIRTY         (1ULL<<6)
#define MMU_PT_FLAG_PSE           (1ULL<<7)
#define MMU_PT_FLAG_GLOBAL        (1ULL<<8)

#define MMU_PT_FLAG_PAT           (1ULL<<12)
#define MMU_PT_FLAG_NX            (1ULL<<63)

#define MMU_PAGE_SIZE 4096
#define MMU_HUGE_PAGE_SIZE_NO_PAE (4*1024*1024)
#define MMU_HUGE_PAGE_SIZE_PAE (2*1024*1024)

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

struct boot_info_t;
typedef struct boot_info_t boot_info_t;

void mmu_init_map(memory_info_t* mem_info);
memory_info_t* mmu_get_memory_map();
void mmu_init_pages(void* addr, bool pae);

void mmu_init(boot_info_t* info, void* buffer, bool use_pae);
void mmu_enable();
bool mmu_get_status();

void mmu_mmap(u64 vaddr, u64 paddr, u64 flags);
void mmu_mmapn(u64 addr, u64 offset, u64 n, u64 flags);

u64 mmu_munmap(u64 vaddr);
u64 mmu_munmapn(u64 vaddr, u64* n);

void mmu_view_map();
void mmu_sort_regions();
void mmu_create_region(u64 base, u64 length, u32 type);
void mmu_merge_region();
void* mmu_search_memrange(
    usize paddr_start, usize size, 
    usize step, u64 flags,
    bool (*criterion)(usize paddr)
);

u64 mmu_byte_to_page_count(u64 size_bytes);
u64 mmu_get_memory_size();

const u32 mmu_get_reserved_mem();
const u32 mmu_get_used_mem();
const u32 mmu_get_free_mem();