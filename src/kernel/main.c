#include "stdint.h"
#include "stdio.h"
#include "string/string.h"
#include "boot.h"
#include "paging/paging.h"
#include "paging/page_allocator.h"
#include "paging/page_table_manager.h"
#include "hal/hal.h"
#include "scheduling/timer.h"


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

    u32 heap_page_count = page_convert_from_bytes(page_get_free_mem()/4);
    memory_init_alloc(0x500000, heap_page_count);

    timer_init();
    while (1)
    {
        sleep(10);
        kprintf("Hey!\n");
    }
    for (;;);
}