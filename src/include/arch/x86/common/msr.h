#pragma once
#include <libk/stdint.h>

bool __attribute__((cdecl)) _x86_has_cpuid();
void __attribute__((cdecl)) _x86_cpuid(u32 leaf, u32 sub, u32* eax, u32* ebx, u32* ecx, u32* edx);
u64 __attribute__((cdecl)) _x86_rdmsr(u32 ecx);
void __attribute__((cdecl)) _x86_wrmsr(u32 ecx, u64 msr);