#pragma once
#include "../stdint.h"

typedef struct
{
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 _unused[23];
} __attribute__((packed)) tss_entry_t;

extern tss_entry_t tss_entry;
void TSS_init();