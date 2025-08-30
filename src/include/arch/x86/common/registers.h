#pragma once
#include <libk/stdint.h>

#ifdef __x86_64__
typedef struct registers_t
{
    // General-purpose registers (pushed manually)
    u64 r15, r14, r13, r12;
    u64 r11, r10, r9,  r8;
    u64 rsi, rdi, rbp, rdx;
    u64 rcx, rbx, rax;

    // Interrupt metadata (pushed manually in your ISR stub)
    u64 vector;       // Interrupt vector number
    u64 error_code;   // Only for exceptions that push error codes (else set to 0)

    // CPU-pushed state (hardware)
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((packed)) registers_t;
#else
typedef struct registers_t
{
    u32 ds;
    u32 edi, esi,  ebp, _, ebx, edx, ecx, eax;
    u32 vector, error;
    u32 eip, cs, eflags, esp, ss; 
} __attribute__((packed)) registers_t;
#endif

void __attribute__((cdecl)) _x86_halt();
void __attribute__((cdecl)) _x86_enable_interrupt();
void __attribute__((cdecl)) _x86_disable_interrupt();