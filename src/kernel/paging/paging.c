#include "paging.h"
#include "page_allocator.h"
#include "page_table_manager.h"
#include "../string/string.h"

void paging_init(boot_info_t* boot_inf)
{
    page_alloc_init();
    u32 kernel_size = ((u32)&__end)-((u32)&__start);
    u32 kernel_pages = page_convert_from_bytes(kernel_size);
    page_alloc_lockn((u32)&__start, kernel_pages);
    
    u32 pd_a = page_alloc_request();
    memset((void*)pd_a, 0, 0x1000);
    page_manager_init(pd_a);

    u32 mem_size=memory_get_total_size_bytes();
    for (u32 i=0;i<mem_size;i+=0x1000)
        page_manager_map_memory(i, i);

    page_alloc_lockn(boot_inf->framebuffer->base, page_convert_from_bytes(boot_inf->framebuffer->size));
    for (u32 i=boot_inf->framebuffer->base; i<boot_inf->framebuffer->base+boot_inf->framebuffer->size;i+=0x1000)
        page_manager_map_memory(i,i);

    paging_load(pd_a);
    paging_enable();
}