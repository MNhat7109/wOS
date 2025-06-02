#include "stdint.h"
#include "stdio.h"
#include "string/string.h"
#include "boot.h"
#include "paging/paging.h"
#include "paging/page_allocator.h"
#include "paging/page_table_manager.h"
#include "hal/hal.h"


void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));
    video_init(boot_inf->framebuffer, boot_inf->font_out);
    kprintf("Hello from kernel!\n");
    // Set up GDT, IDT, ISRs, IRQs
    HAL_init();
    memory_init(boot_inf->mem_map);
    memory_view_map();
    paging_init(boot_inf);

    kprintf("Used memory: %u KB\n", page_get_used_mem()>>10);
    kprintf("Reserved memory: %u KB\n", page_get_reserved_mem()>>10);
    kprintf("Free memory: %u KB\n", page_get_free_mem()>>10);
    
    u32 heap_page_count = page_convert_from_bytes(page_get_free_mem()/4);

    memory_init_alloc(0x500000, heap_page_count);
    u8* ptr = memory_allocate(8192);
    memory_free(ptr);
    memory_destroy_alloc();


    kprintf("Used memory: %u KB\n", page_get_used_mem()>>10);
    kprintf("Reserved memory: %u KB\n", page_get_reserved_mem()>>10);
    kprintf("Free memory: %u KB\n", page_get_free_mem()>>10);

    for (;;);
}