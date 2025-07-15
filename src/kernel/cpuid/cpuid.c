#include "cpuid.h"
#include "../x86/x86.h"

u64 cpuid(u32 eax)
{
    return _x86_get_cpuid(eax);
}

u64 rdmsr(u32 ecx)
{
    return _x86_rdmsr(ecx);
}

void wrmsr(u32 ecx, u64 msr)
{
    _x86_wrmsr(ecx, msr);
}