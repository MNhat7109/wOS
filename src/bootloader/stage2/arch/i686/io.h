#pragma once
#include "../stdint.h"

void __attribute__((cdecl)) i686_outb(u16 port, u8 value);
u8 __attribute__((cdecl)) i686_inb(u16 port);
void __attribute__((cdecl)) i686_outl(u16 port, u32 value);
u32 __attribute__((cdecl)) i686_inl(u16 port);
void __attribute__((cdecl)) i686_outsl(u16 port, u8 selector, void* address, u32 count);
void __attribute__((cdecl)) i686_insl(u16 port, u8 selector, void* address_out, u32 count);
void __attribute__((cdecl)) i686_outsw(u16 port, u8 selector, void* address, u32 count);
void __attribute__((cdecl)) i686_insw(u16 port, u8 selector, void* address_out, u32 count);

#define inb(_p) _x86_inb((_p))
#define outb(_p, _v) _x86_outb((_p), (_v))
#define inl(_p) _x86_inl((_p))
#define outl(_p, _v) _x86_outl((_p), (_v))
#define insl(_p, _a, _c) _x86_insl((_p),0x10, (_a), (_c))
#define outsl(_p, _a, _c) _x86_outsl((_p),0x10, (_a), (_c))
#define insw(_p, _a, _c) _x86_insw((_p),0x10, (_a), (_c))
#define outsw(_p, _a, _c) _x86_outsw((_p),0x10, (_a), (_c))
#define insw_far(_p, _s, _a, _c) _x86_insw((_p),(_s), (_a), (_c))
#define outsw_far(_p, _s, _a, _c) _x86_outsw((_p),(_s), (_a), (_c))