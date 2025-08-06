#pragma once
#include "../stdint.h"

void __attribute__((cdecl)) _x86_GDT_load(void* gdtr, u16 cs, u16 ds);
void __attribute__((cdecl)) _x86_IDT_load(void* idtr);
void __attribute__((cdecl)) _x86_TSS_flush();
void __attribute__((cdecl)) _x86_TSS_save_esp0(u32* esp0);
void __attribute__((cdecl)) _x86_panic();
void __attribute__((cdecl)) _x86_halt();
void __attribute__((cdecl)) _x86_enable_interrupt();
void __attribute__((cdecl)) _x86_disable_interrupt();
u64 __attribute__((cdecl)) _x86_get_cpuid(u32 eax);
u64 __attribute__((cdecl)) _x86_rdmsr(u32 ecx);
void __attribute__((cdecl)) _x86_wrmsr(u32 ecx, u64 msr);
void __attribute__((cdecl)) _x86_cpu_check(u8*, u8*);
void __attribute__((cdecl)) crash();

void __attribute__((cdecl)) _x86_outb(u16 port, u8 value);
u8 __attribute__((cdecl)) _x86_inb(u16 port);

void __attribute__((cdecl)) _x86_outw(u16 port, u16 value);
u16 __attribute__((cdecl)) _x86_inw(u16 port);

void __attribute__((cdecl)) _x86_outl(u16 port, u32 value);
u32 __attribute__((cdecl)) _x86_inl(u16 port);

void __attribute__((cdecl)) _x86_load_paging(u32 address);
void __attribute__((cdecl)) _x86_enable_paging();
void __attribute__((cdecl)) _x86_tlb_flush(u32 virtual_address);

void __attribute__((cdecl)) _x86_multitasking_save_regs(u32* esp, u32* cr3);
void __attribute__((cdecl)) _x86_multitasking_switch_task(void* next_task);