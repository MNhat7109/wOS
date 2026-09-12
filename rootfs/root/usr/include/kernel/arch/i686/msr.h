#pragma once
#include <stdint.h>
#include <stdbool.h>

bool msr_check();
void __attribute__((cdecl)) msr_write(u32 msr, u64 value); 
u64 __attribute__((cdecl)) msr_read(u32 msr); 