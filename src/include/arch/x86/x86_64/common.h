#pragma once
#include <libk/stdint.h>

void x86_64_GDT_load(void* gdtr, u16 cs, u16 ds);
void x86_64_IDT_load(void* idtr);
void x86_64_TSS_flush();
void x86_64_TSS_save_stack(u64* stack_ptr);
