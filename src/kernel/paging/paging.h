#pragma once
#include "../stdint.h"
#include "../x86/x86.h"
#include "../boot.h"

#define paging_load(_a) _x86_load_paging((_a)) 
#define paging_enable() _x86_enable_paging()
#define paging_tlb_flush(_a) _x86_tlb_flush((_a))

void paging_init(boot_info_t* boot_inf);