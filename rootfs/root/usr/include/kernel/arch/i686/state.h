#pragma once
#include <stdint.h>

typedef struct register_state_t
{
    u32 ds;
    u32 edi, esi,  ebp, _, ebx, edx, ecx, eax;
    u32 vector, error;
    u32 eip, cs, eflags, esp, ss; 
} __attribute__((packed)) register_state_t;

