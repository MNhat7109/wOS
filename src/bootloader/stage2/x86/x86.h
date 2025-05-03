#pragma once
#include "../stdint.h"

void __attribute__((cdecl)) _x86_outb(u16 port, u8 value);
u8 __attribute__((cdecl)) _x86_inb(u16 port);
void __attribute__((cdecl)) _x86_outl(u16 port, u32 value);
u32 __attribute__((cdecl)) _x86_inl(u16 port);
void __attribute__((cdecl)) _x86_outsl(u16 port, u8 selector, void* address, u32 count);
void __attribute__((cdecl)) _x86_insl(u16 port, u8 selector, void* address_out, u32 count);
void __attribute__((cdecl)) _x86_outsw(u16 port, u8 selector, void* address, u32 count);
void __attribute__((cdecl)) _x86_insw(u16 port, u8 selector, void* address_out, u32 count);

void __attribute__((cdecl)) _x86_idt_load(void* idtr);
void __attribute__((cdecl)) _x86_panic();
void __attribute__((cdecl)) _x86_enable_interrupt();
void __attribute__((cdecl)) _x86_disable_interrupt();
void __attribute__((cdecl)) _x86_halt();