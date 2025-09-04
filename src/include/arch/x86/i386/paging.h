#pragma once
#include <libk/stdint.h>

void __attribute__((cdecl)) i386_load_paging(u32 address);
void __attribute__((cdecl)) i386_enable_paging();
void __attribute__((cdecl)) i386_enable_pae();
void __attribute__((cdecl)) i386_tlb_flush(u32 virtual_address);
void __attribute__((cdecl)) i386_read_cr2(u32* cr2);