#pragma once
#include <libk/stdint.h>

void __attribute__((cdecl)) _x86_outb(u16 port, u8 value);
u8 __attribute__((cdecl)) _x86_inb(u16 port);

void __attribute__((cdecl)) _x86_outw(u16 port, u16 value);
u16 __attribute__((cdecl)) _x86_inw(u16 port);

void __attribute__((cdecl)) _x86_outl(u16 port, u32 value);
u32 __attribute__((cdecl)) _x86_inl(u16 port);