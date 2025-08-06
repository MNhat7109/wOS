#pragma once
#include "../../stdint.h"
#include <stdbool.h>

typedef struct 
{
    void (*wrmsr)(u32, u64);
    u64 (*rdmsr)(u32);
} __attribute__((packed)) cpu_io_t;

const cpu_io_t cio_load_defaults();
void cio_load_msr();

void cpu_feature_check(u8*, u8*);