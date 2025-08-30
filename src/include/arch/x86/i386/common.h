#pragma once
#include <libk/stdint.h>

void __attribute__((cdecl)) i386_GDT_load(void* gdtr, u16 cs, u16 ds);
void __attribute__((cdecl)) i386_IDT_load(void* idtr);
void __attribute__((cdecl)) i386_TSS_flush();
void __attribute__((cdecl)) i386_TSS_save_sp(u32* stack_ptr);