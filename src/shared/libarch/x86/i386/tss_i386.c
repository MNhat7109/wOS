#ifdef __i386__

#include <arch/x86/common/tss.h>
#include <arch/x86/i386/common.h>
#include <arch/x86/common/gdt.h>

typedef struct
{
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 _unused[23];
} __attribute__((packed)) tss_entry_t;

tss_entry_t tss_entry;

void x86_TSS_set_stack(u16 ss, usize sp)
{
    tss_entry.ss0 = ss;
    tss_entry.esp0 = sp;

    i386_TSS_flush();
}

void x86_TSS_init(u32 index)
{
    u32 base = (u32)&tss_entry;
    u32 limit = sizeof(tss_entry)-1;

    x86_GDT_set_attr(
        index, 
        (void*)base, 
        limit, 
        (GDT_ACCESS_PVL_KRNL | GDT_ACCESS_TSS32_AVL),
        0
    );
    x86_GDT_mark_present(index);
}

#endif