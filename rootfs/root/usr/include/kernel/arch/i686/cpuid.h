#pragma once
#include <stdint.h>
#include <stdbool.h>

bool __attribute__((cdecl)) cpuid_check();
void __attribute__((cdecl)) cpuidex(u32 func, u32 subfunc, u32 cpu_info_out[4]);
void __attribute__((cdecl)) cpuid(u32 func, u32 cpu_info_out[4]);