#pragma once
#include <libk/stdint.h>

void x86_64_load_paging(u64 address);
void x86_64_enable_paging();
void x86_64_tlb_flush(u64 virtual_address);