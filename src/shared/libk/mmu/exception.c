#include <libk/mmu/exception.h>

#include <libk/interrupt/isr.h>

#if defined(__i386__) || defined(__x86_64__)
#include <arch/x86/utils/mmu/handlers.h>
#endif


#define MMU_PF_VECTOR 0xE
#define MMU_GPF_VECTOR 0xD

void mmu_exception_init()
{
    // We register both handlers with zero additional details whatsoever
    // If the clients want to do so, they can reregister either or both of the handlers.
    ISR_reg_handler(MMU_GPF_VECTOR, mmu_general_protection_fault_handler, NULL);
    ISR_reg_handler(MMU_PF_VECTOR, mmu_page_fault_handler, NULL);
}

void mmu_exception_register_ctx_pf(void* ctx)
{
    ISR_reg_handler(MMU_PF_VECTOR, mmu_page_fault_handler, ctx);
}

void mmu_exception_register_ctx_gpf(void* ctx)
{
    ISR_reg_handler(MMU_GPF_VECTOR, mmu_general_protection_fault_handler, ctx);
}