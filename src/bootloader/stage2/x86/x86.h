#pragma once
#include "../stdint.h"

void __attribute__((cdecl)) _x86_outb(u16 port, u8 value);
u8 __attribute__((cdecl)) _x86_inb(u16 port);
void __attribute__((cdecl)) _x86_idt_load(void* idtr);
void __attribute__((cdecl)) _x86_panic();
void __attribute__((cdecl)) _x86_enable_interrupt();