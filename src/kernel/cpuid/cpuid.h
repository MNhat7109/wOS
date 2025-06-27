#pragma once
#include "../stdint.h"

u64 rdmsr(u32 ecx);
void wrmsr(u32 ecx, u64 msr);
u64 cpuid(u32 eax);