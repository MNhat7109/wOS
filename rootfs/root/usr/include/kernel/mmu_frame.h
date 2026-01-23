#pragma once
#include <stdint.h>

void mmu_frame_init(u8* bmp_buffer_addr, u64 mem_size);

void mmu_frame_set(uptr address);
void mmu_frame_reserve(uptr address);
void mmu_frame_clear(uptr address);

void mmu_frame_set_n(uptr address, usize n);
void mmu_frame_clear_n(uptr address, usize n);
void mmu_frame_reserve_n(uptr address, usize n);

uptr mmu_frame_next();