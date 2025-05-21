#pragma once
#include "../x86/x86.h"

#define inb(_p) _x86_inb((_p))
#define outb(_p, _v) _x86_outb((_p), (_v))

void iowait();