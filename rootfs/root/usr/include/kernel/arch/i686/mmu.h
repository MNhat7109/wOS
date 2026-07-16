#pragma once

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
} mmu_i386_page_attributes_t;
