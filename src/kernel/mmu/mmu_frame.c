#include <kernel/mmu_frame.h>



void mmu_frame_init(uptr start_addr, usize mem_size)
{

}

void mmu_frame_set(uptr address);
void mmu_frame_clear(uptr address);

void mmu_frame_set_n(uptr address, usize n);
void mmu_frame_clear_n(uptr address, usize n);

uptr mmu_frame_next();