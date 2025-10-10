#include <libk/mmu/mmu_frame.h>
#include <libk/mmu/mmu_map.h>
#include <libk/mmu/mmu.h>

static struct
{
    void* start_addr;
} mmu_frame_data;

void mmu_frame_init(void* addr)
{
    mmu_frame_data.start_addr = addr;
    // Carve out the regions we need for the buddy system

    memory_info_t* mem_map = mmu_map_get();

    u32 entry_count = mem_map->entries_count;
    for (u32 i=0;i<entry_count;i++)
    {
        memory_region_t* current = &mem_map->regions[i];
        if (current->type == MEMORY_TYPE_FREE)
        {
            buddy_alloc_init(current->base, current->length);
        }
    }
}

void mmu_frame_set(usize size, int type)
{

}

u64 mmu_frame_create(usize size);
void mmu_frame_free(u64 addr);

usize mmu_frame_get_size(int type);