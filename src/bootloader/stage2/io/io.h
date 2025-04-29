#pragma once
#include "../x86/x86.h"

#define inb(_p) _x86_inb((_p))
#define outb(_p, _v) _x86_outb((_p), (_v))

// u8 inb(u16 port);
// void outb(u16 port, u8 value);

void iowait();