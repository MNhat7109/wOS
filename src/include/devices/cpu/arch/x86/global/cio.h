#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

typedef struct cio_layer_t
{
    void (*cpuid)(u32 leaf, u32 sub, u32* a, u32* b, u32* c, u32* d);
    void (*wrmsr)(u32 ecx, u64 value);
    u64 (*rdmsr)(u32 ecx);
} cio_layer_t;

const cio_layer_t* cio_load_defaults();