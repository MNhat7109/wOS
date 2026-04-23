#pragma once
#include <stdint.h>
#include <kernel/mmu_frame.h>

#define MODULE_MMU "MMU"

#define PAGE_SIZE (1<<12)
usize HUGE_PAGE_SIZE();

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

typedef struct memory_info_t
{
    u32 entries_count;
    memory_region_t regions[];
} __attribute__((packed)) memory_info_t;

typedef enum
{
    MMU_PG_ATTR_PRESENT = (1ULL<<0), // Present
    MMU_PG_ATTR_RW = (1ULL<<1), // Read-write
    MMU_PG_ATTR_US = (1ULL<<2), // User-super
    MMU_PG_ATTR_PWT = (1ULL<<3), // Write-through
    MMU_PG_ATTR_PCD = (1ULL<<4), // Uncacheable
    MMU_PG_ATTR_ACCESSED = (1ULL<<5), // Accessed indicator
    MMU_PG_ATTR_GLOBAL = (1ULL<<6), // No invalidate
    MMU_PG_ATTR_PSE = (1ULL<<7), // Huge page
    MMU_PG_ATTR_PAT = (1ULL<<12), // Page attribute table
    MMU_PG_ATTR_NX = (1ULL<<63), // Execute disable
} mmu_page_attributes_t;

typedef enum
{
    MMU_FLAG_MAP_ID = (1<<0),
    MMU_FLAG_HUGE_PAGE = (1<<1),
    MMU_FLAG_VERY_HUGE_PAGE= (1<<2),
} mmu_flags_t;

typedef uptr vaddr_t;
typedef u64 paddr_t;

extern mmu_frame_allocator_t* mmu_frame_alloc;

#define mmu_align_down(_x, _a) ((_x) & ~((_a) - 1))
#define mmu_align_up(_x, _a)   (((_x) + (_a) - 1) & ~((_a) - 1))
#define mmu_is_aligned(_x, _a) (((_x) & ((_a) - 1)) == 0)

#define mmu_byte_to_4k_pages(_n) (((_n)+((1<<12)-1)) >> 12)
#define mmu_byte_to_4m_pages(_n) (((_n)+((1<<22)-1)) >> 22)
#define mmu_byte_to_2m_pages(_n) (((_n)+((1<<21)-1)) >> 21)
#define mmu_byte_to_1g_pages(_n) (((_n)+((1<<30)-1)) >> 30)

paddr_t mmu_vtop(vaddr_t vaddr);
vaddr_t mmu_ptov(paddr_t paddr);

int mmu_init(uptr start_addr, memory_info_t* mem_map);
void mmu_init_stage2();

void mmu_load_address_space(paddr_t paddr);
void mmu_reload_address_space();
void mmu_enable_features();
void mmu_mmap(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmapn(paddr_t addr, usize n, u64 attributes, int flags);
void mmu_mmap_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);
void mmu_mmap_very_huge(vaddr_t vaddr, paddr_t paddr, u64 attributes);

usize mmu_munmap(vaddr_t vaddr, paddr_t* paddr_out);
usize mmu_munmapn(vaddr_t vaddr, paddr_t* first_paddr_out, usize n);