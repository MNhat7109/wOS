#pragma once
#include <libk/stdint.h>

void mmu_exception_init();

void mmu_exception_register_ctx_pf(void* ctx);
void mmu_exception_register_ctx_gpf(void* ctx);
