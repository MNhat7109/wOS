#pragma once
#include <stdint.h>

#define PAGE_SIZE (1<<12)
static inline HUGE_PAGE_SIZE();

#define HUGE_PAGE_SIZE_NO_PAE (1<<22)
#define HUGE_PAGE_SIZE_PAE (1<<21)

#ifdef __x86_64__
#define VERY_HUGE_PAGE_SIZE (1<<30)
#endif

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

typedef enum
{
    MMU_PA_FLAG_PRESENT = (1ULL<<0), // Present
    MMU_PA_FLAG_RW = (1ULL<<1), // Read-write
    MMU_PA_FLAG_US = (1ULL<<2), // User-super
    MMU_PA_FLAG_PWT = (1ULL<<3), // Write-through
    MMU_PA_FLAG_PCD = (1ULL<<4), // Uncacheable
    MMU_PA_FLAG_ACCESSED = (1ULL<<5), // Accessed indicator
    MMU_PA_FLAG_GLOBAL = (1ULL<<6), // No invalidate
    MMU_PA_FLAG_PSE = (1ULL<<7), // Huge page
    MMU_PA_FLAG_PAT = (1ULL<<12), // Page attribute table
    MMU_PA_FLAG_NX = (1ULL<<63), // Execute disable
} mmu_page_attributes_t;

typedef enum
{
    MMU_MAP_ID_ENABLE = (1<<0),
    MMU_HUGE_PAGE = (1<<1),
    MMU_VERY_HUGE_PAGE= (1<<2),
} mmu_flags_t;

typedef uptr vaddr_t;
typedef u64 paddr_t;

#define mmu_byte_to_4k_pages(_n) (((_n) >> 12)+((_n)&((1<<12)-1)))
#define mmu_byte_to_4m_pages(_n) (((_n) >> 22)+((_n)&((1<<22)-1)))
#define mmu_byte_to_2m_pages(_n) (((_n) >> 21)+((_n)&((1<<21)-1)))
#define mmu_byte_to_1g_pages(_n) (((_n) >> 30)+((_n)&((1<<30)-1)))

static inline paddr_t mmu_vtop(vaddr_t vaddr);
static inline vaddr_t mmu_ptov(paddr_t paddr);

void mmu_init(memory_info_t* mem_map);
void mmu_mmap(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmapn(paddr_t addr, usize n, u64 attributes, int flags);
void mmu_mmap_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmap_very_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);

usize mmu_munmap(vaddr_t vaddr, paddr_t* paddr_out);
usize mmu_munmapn(vaddr_t vaddr, paddr_t* first_paddr_out, usize n);
