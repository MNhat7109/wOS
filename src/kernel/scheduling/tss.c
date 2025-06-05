#include "tss.h"
#include "../hal/gdt/gdt.h"
#include "../x86/x86.h"

tss_entry_t tss_entry;

#define TSS_flush() _x86_TSS_flush() 

#define TSS_save_stack_ring0(_s0) _x86_TSS_save_esp0((_s0))

void TSS_init()
{
    u32 base = (u32)&tss_entry;
    u32 limit = sizeof(tss_entry)-1;

    GDT_set_attr(&gdt[5], base, limit, 
        (GDT_ACCESS_PVL_KRNL | GDT_ACCESS_TSS32_AVL), 0);
    GDT_mark_present(&gdt[5]);

    tss_entry.ss0 = KERNEL_DATA_SEG;
    TSS_save_stack_ring0(&tss_entry.esp0);
    TSS_flush();
}