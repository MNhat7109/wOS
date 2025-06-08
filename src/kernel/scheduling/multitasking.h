#pragma once
#include "../stdint.h"
#include "../x86/x86.h"

typedef struct
{
    u8 state;
    u32 stack; // Save the address of ESP in here.
    u32 stack_ring0;
    u32 cr3;
    struct proc_ctrl_block_t* next;
} proc_ctrl_block_t;

extern proc_ctrl_block_t* current_process;


void multitasking_init();
void multitasking_switch(proc_ctrl_block_t* next_task);
void multitasking_create(void* task_fptr);
void multitasking_schedule();
// TODO: Pre-emptive multitask (round robin)