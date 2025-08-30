#pragma once
#include <libk/stdint.h>

void x86_TSS_init(u32 index);
void x86_TSS_set_stack(u16 ss, usize sp);