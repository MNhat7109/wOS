#pragma once
#include <stdint.h>

int mmio_setup_addr(void* addr, usize size);
void mmio_outl(volatile u32* base, u32 offset, u32 value);
void mmio_outq(volatile u64* base, u32 offset, u64 value);
u32 mmio_inl(volatile u32* base, u32 offset);
u64 mmio_inq(volatile u64* base, u32 offset);