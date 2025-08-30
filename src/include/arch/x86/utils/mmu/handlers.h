#pragma once

struct registers_t;
typedef struct registers_t registers_t;

void mmu_general_protection_fault_handler(registers_t* regs, void* ctx);
void mmu_page_fault_handler(registers_t* regs, void* ctx);

