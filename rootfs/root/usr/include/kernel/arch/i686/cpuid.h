#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    CPUID_FUNC_GETVENDORSTRING,
    CPUID_FUNC_GETFEATURES,
    CPUID_FUNC_GETTLB,
    CPUID_FUNC_GETSERIAL,

    CPUID_FUNC_X_MAXLEAF=0x80000000,
    CPUID_FUNC_X_GETFEATURES,
    CPUID_FUNC_X_GETBRANDSTRING,
    CPUID_FUNC_X_GETBRANDSTRINGMORE,
    CPUID_FUNC_X_GETBRANDSTRINGEND,
} cpuid_functions_t;

bool __attribute__((cdecl)) cpuid_check();
void __attribute__((cdecl)) cpuidex(u32 func, u32 subfunc, u32 cpu_info_out[4]);
void __attribute__((cdecl)) cpuid(u32 func, u32 cpu_info_out[4]);