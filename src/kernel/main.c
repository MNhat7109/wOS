#include "stdint.h"
#include "stdio.h"
#include "string/string.h"
#include "boot.h"
#include "paging/paging.h"
#include "paging/page_allocator.h"
#include "paging/page_table_manager.h"
#include "hal/hal.h"
#include "scheduling/timer.h"
#include "scheduling/tss.h"
#include "scheduling/multitasking.h"

void process_A();
void process_B();
void process();

void __attribute__((section(".entry"))) start(boot_info_t* boot_inf)
{
    memset(&__bss_start, 0, (&__end)-(&__bss_start));

    // Set up display
    video_init(boot_inf->framebuffer, boot_inf->font_out);
    kprintf("Hello from kernel!\n");

    // Set up GDT, IDT, ISRs, IRQs
    HAL_init();

    // Set up memory
    memory_init(boot_inf->mem_map);
    memory_view_map();
    paging_init(boot_inf);
    u32 heap_page_count = page_convert_from_bytes(page_get_free_mem()/4);
    memory_init_alloc(0x500000, heap_page_count);

    // Set up scheduling for multitasking
    timer_init();
    TSS_init();
    // multitasking_init(0x500000);
    kprintf("%x\n", start);
    // current_process->stack-=4;
    // *((u32*)current_process->stack) = (u32)process;
    
    multitasking_create(process_A, 0);
    // multitasking_create(process_B, 0);

    multitasking_schedule();
    // _x86_panic();
    // process();
    //multitasking_switch(A);
    // kprintf("")
    for (;;);
}

void process()
{
    for (;;);
}

void process_A()
{
    // sleep(100);
    kprintf("We're now in process_A\n");
    // multitasking_schedule();
    kprintf("Yo now we're here again!\n");
    for (;;);
}

void process_B()
{
    // sleep(100);
    kprintf("We're now in process_B\n");
    multitasking_schedule();
    for (;;);
    // ((u32*)A->stack)[0] = (u32)process_A;
    // kprintf("%x\n", ((u32*)A->stack)[0]);
}