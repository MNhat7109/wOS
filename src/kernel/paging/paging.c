#include "paging.h"

#include <stdbool.h>
#include "../string/string.h"
#include "../memory/memory.h"
#include "../video/video.h"

u32 kernel_page_dir;
bool mmu_on=false;

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

    framebuffer_t* fb_ptr = (framebuffer_t*)boot_inf->framebuffer;
    page_alloc_lockn(fb_ptr->base, page_convert_from_bytes(fb_ptr->size));
    for (u32 i=fb_ptr->base; i<fb_ptr->base+fb_ptr->size;i+=0x1000)
        page_manager_map_memory(i,i);

    paging_load(kernel_page_dir);
    paging_enable();

    mmu_on=true;
}

bool paging_is_mmu_on()
{
    return mmu_on;
}