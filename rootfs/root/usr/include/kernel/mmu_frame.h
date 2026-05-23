#pragma once
#include <stdint.h>

typedef struct mmu_frame_allocator_t mmu_frame_allocator_t;
typedef struct mmu_frame_plugins_t mmu_frame_plugins_t;

typedef int (*mmu_frame_init_cb_t)(mmu_frame_allocator_t* this, uptr offset, u64 size);

typedef struct mmu_frame_allocator_ops_t
{
    uptr (*alloc)(mmu_frame_allocator_t* f_alloc, u64 n);
    void (*free)(mmu_frame_allocator_t* f_alloc, uptr phys_addr);
    void (*reserve_pages)(mmu_frame_allocator_t* f_alloc, uptr paddr, u64 n);
    void (*lock_pages)(mmu_frame_allocator_t* f_alloc, uptr paddr, u64 n);
    void (*release_pages)(mmu_frame_allocator_t* f_alloc, uptr paddr, u64 n);
} mmu_frame_allocator_ops_t;

typedef struct mmu_frame_allocator_t
{
    uptr meta_offset_vaddr;
    u64 meta_size;
    mmu_frame_plugins_t* plugin;
} mmu_frame_allocator_t;

typedef struct mmu_frame_plugins_t
{
    mmu_frame_init_cb_t init;
    mmu_frame_allocator_ops_t ops;
} mmu_frame_plugins_t;

uptr mmu_frame_get_meta_offset();
u64 mmu_frame_get_meta_size();

int mmu_frame_populate(mmu_frame_plugins_t* plugin, uptr offset, u64 size);
uptr mmu_frame_alloc(u64 n);
void mmu_frame_free(uptr paddr);
void mmu_frame_reserve_pages(uptr paddr, u64 n);
void mmu_frame_lock_pages(uptr paddr, u64 n);
void mmu_frame_release_pages(uptr paddr, u64 n);