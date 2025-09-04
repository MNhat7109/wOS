#include <libk/stdio.h>
#include <libk/mmu/mmu.h>

#if defined(__i386__)
#include <arch/x86/utils/mmu/handlers.h>
#include <arch/x86/utils/kernel/kernel.h>

#include <arch/x86/i386/paging.h>
#include <arch/x86/common/registers.h>

#define CODE_SEGMENT_USER 3
#define CODE_SEGMENT_KRNL 0

#define PF_ERRCODE_PAGE_PRESENT (1<<0)
#define PF_ERRCODE_PAGE_RW      (1<<1)
#define PF_ERRCODE_PAGE_US      (1<<2)

typedef struct mmu_handler_ctx
{
    void (*funcp)(void* ctx);
    void* context;
} mmu_handler_ctx;

void mmu_general_protection_fault_handler(registers_t* regs, void* ctx)
{
    kprintf("MMU: Memory access violation detected\n");
    kprintf("Error code: 0x%x\n", regs->error);
    kprintf("At EIP=0x%x\n", regs->eip);

    // Check if the offence happened in userspace
    if ((regs->cs & 0x3) == CODE_SEGMENT_USER)
    {
        if (!ctx) 
        {
            kprintf("MMU: Cannot find a handler to handle additional calls. System will proceed without one\n");
            return;
        }
        mmu_handler_ctx* mmu_ctx = (mmu_handler_ctx*)ctx;
        mmu_ctx->funcp(mmu_ctx->context);
    }
    else
    {
        // Panic
        kprintf("MMU: Violation happened in kernel space, cannot continue. Dying...\n");
        _x86_disable_interrupt();
        _x86_halt();
    }
}

void mmu_page_fault_handler(registers_t* regs, void* ctx)
{
    u32 cr2;
    i386_read_cr2(&cr2);

    kprintf("MMU: Page fault detected\n");
    kprintf("Error code: 0x%x\n", regs->error);
    kprintf("addr=0x%x\n", cr2);

    if (!(regs->error & PF_ERRCODE_PAGE_PRESENT))
    {
        // Lazy allocation
        mmu_mmap(cr2, cr2, MMU_PT_FLAG_READ_WRITE | MMU_PT_FLAG_USER_SUPER);
    }
}

#endif