#include "page_table_manager.h"
#include "../string/string.h"
#include "paging.h"

#define PAGE_TABLE_FLAG_PRESENT       0
#define PAGE_TABLE_FLAG_READ_WRITE    1
#define PAGE_TABLE_FLAG_USER_SUPER    2
#define PAGE_TABLE_FLAG_WRITE_THRU    3
#define PAGE_TABLE_FLAG_CACHE_DISABLE 4
#define PAGE_TABLE_FLAG_ACCESSED      5
#define PAGE_TABLE_FLAG_DIRTY         6
#define PAGE_TABLE_FLAG_PAT           7
#define PAGE_TABLE_FLAG_GLOBAL        8
#define PAGE_TABLE_FLAG_AVL           9

u32* page_directory;

u8 page_manager_get_flag(u32 value, int flag)
{
    return (value & (1<<flag)) >> flag;
}
void page_manager_set_flag(u32* value, int flag)
{
    *value |= (1<<flag);
}
void page_manager_clear_flag(u32* value, int flag)
{
    *value &= ~(1<<flag);
}

void page_manager_init(u32 address)
{
    page_directory = (u32*)address;
}

void page_manager_map_memory(u32 virtual_address, u32 physical_address)
{
    u32 page_index = (virtual_address >> 12) & 0x3FF; 
    u32 page_table_index = (virtual_address >> 22) & 0x3FF; 

    u32* page_table;
    u32 page_dir_entry = page_directory[page_table_index];
    u8 present = page_manager_get_flag(page_dir_entry, PAGE_TABLE_FLAG_PRESENT);
    if (present == 0)
    {
        page_table = (u32*)page_alloc_request();
        memset(page_table, 0, 0x1000);

        page_dir_entry = (u32)page_table & 0xFFFFF000;
        page_manager_set_flag(&page_dir_entry, PAGE_TABLE_FLAG_PRESENT);
        page_manager_set_flag(&page_dir_entry, PAGE_TABLE_FLAG_READ_WRITE);
        page_directory[page_table_index] = page_dir_entry;
    }
    else page_table = (u32*)(page_dir_entry & 0xFFFFF000);

    u32 page_entry = page_table[page_index];
    page_entry = physical_address & 0xFFFFF000;
    page_manager_set_flag(&page_entry, PAGE_TABLE_FLAG_PRESENT);
    page_manager_set_flag(&page_entry, PAGE_TABLE_FLAG_READ_WRITE);
    page_table[page_index] = page_entry;

    paging_tlb_flush(virtual_address);
}

// NOTE: Fixed the unmapping function.
// Prior to fixing, this thing just emits a rotten smell of bugs there.
// It unmaps a page, then free that page, too.
// What if the unmapped page isn't allocated?
u32 page_manager_unmap_memory(u32 virtual_address)
{
    u32 page_index = (virtual_address >> 12) & 0x3FF; 
    u32 page_table_index = (virtual_address >> 22) & 0x3FF; 

    u32 page_dir_entry = page_directory[page_table_index];
    u8 present = page_manager_get_flag(page_dir_entry, PAGE_TABLE_FLAG_PRESENT);
    if (!present) return 0;

    u32* page_table = (u32*)(page_dir_entry & 0xFFFFF000);
    u32 page_entry = page_table[page_index];
    
    present = page_manager_get_flag(page_entry, PAGE_TABLE_FLAG_PRESENT);
    if (!present) return 0;

    page_manager_clear_flag(&page_entry, PAGE_TABLE_FLAG_PRESENT);
    u32 physical_address = page_entry & 0xFFFFF000;

    page_table[page_index] = 0;

    paging_tlb_flush(virtual_address);
    return physical_address; // Returns the address for further freeing.
}