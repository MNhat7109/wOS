#pragma once


#include <stdint.h>

void __attribute__((cdecl)) outb(u16 port, u8 value);
u8 __attribute__((cdecl)) inb(u16 port);
void __attribute__((cdecl)) outw(u16 port, u16 value);
u16 __attribute__((cdecl)) inw(u16 port);
void __attribute__((cdecl)) outl(u16 port, u32 value);
u32 __attribute__((cdecl)) inl(u16 port);
