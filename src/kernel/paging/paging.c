#include "paging.h"
#include "../string/string.h"

u32 kernel_page_dir;

void paging_init(boot_info_t* boot_inf)
{
    page_alloc_init();
    u32 kernel_size = ((u32)&__end)-((u32)&__start);
    u32 kernel_pages = page_convert_from_bytes(kernel_size);
    page_alloc_lockn((u32)&__start, kernel_pages);
    
    kernel_page_dir = page_alloc_request();
    memset((void*)kernel_page_dir, 0, 0x1000);
    page_manager_init(kernel_page_dir);

    u32 mem_size=memory_get_total_size_bytes();
    for (u32 i=0;i<mem_size;i+=0x1000)
        page_manager_map_memory(i, i);

    page_alloc_lockn(boot_inf->framebuffer->base, page_convert_from_bytes(boot_inf->framebuffer->size));
    for (u32 i=boot_inf->framebuffer->base; i<boot_inf->framebuffer->base+boot_inf->framebuffer->size;i+=0x1000)
        page_manager_map_memory(i,i);

    paging_load(kernel_page_dir);
    paging_enable();
}