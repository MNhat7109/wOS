#include "multitasking.h"
#include "../memory/memory.h"
#include "../stdio.h"
#include "../string/string.h"
#include "tss.h"

#define TASK_STATE_TERMINATED 0
#define TASK_STATE_RUNNING 1
#define TASK_STATE_SUSPENDED 2

#define MAX_STACK_SPACE 8192

proc_ctrl_block_t* current_process = NULL;
proc_ctrl_block_t* last = NULL;


#define multitasking_save_regs(_esp, _cr3) _x86_multitasking_save_regs((_esp), (_cr3))

void multitasking_switch(proc_ctrl_block_t* next_task)
{
    _x86_multitasking_switch_task(next_task);
}

void multitasking_create(void* task_fptr, u32 heap_address)
{
    proc_ctrl_block_t* new_proc = (proc_ctrl_block_t*)memory_allocate(sizeof(proc_ctrl_block_t));
    if (!new_proc)
    {
        kprintf("Mutitasking: Creating process crashed unexpectedly\n");
        return ;
    }
    new_proc->state= TASK_STATE_RUNNING;
    u8* container = (u8*)memory_allocate(MAX_STACK_SPACE);
    memset(container, 0, MAX_STACK_SPACE);
    
    new_proc->stack = (u32)(container+MAX_STACK_SPACE);
    new_proc->stack-=sizeof(u32);
    *((u32*)new_proc->stack) = (u32)task_fptr;

    new_proc->stack_ring0 = new_proc->stack;
    new_proc->heap_addr = heap_address;
    new_proc->cr3 = current_process->cr3; // TODO: Adding functionalities to this so that
                                        // each program has its own VAS
    
    new_proc->next = last->next;
    last->next = (struct proc_ctrl_block_t*)new_proc;
    last = (proc_ctrl_block_t*)new_proc;
}

void multitasking_init(u32 kernel_heap_address)
{
    current_process = (proc_ctrl_block_t*)memory_allocate(sizeof(proc_ctrl_block_t));
    if (!current_process) return;
    current_process->heap_addr = kernel_heap_address;
    current_process->state = TASK_STATE_RUNNING;
    current_process->next = (struct proc_ctrl_block_t*)current_process;
    current_process->stack_ring0 = tss_entry.esp0;

    last = current_process;

    multitasking_save_regs(&current_process->stack, &current_process->cr3);
    kprintf("%x\n", current_process);
    kprintf("%x, %x, %x\n", current_process->stack, current_process->stack_ring0, current_process->cr3);
    
    // current_process->stack-=4;
    // *(u32*)current_process->stack = (u32)&&e;
 }

void multitasking_schedule()
{
    // kprintf("%x\n", multitasking_schedule);
    multitasking_switch((proc_ctrl_block_t*)current_process->next);
}