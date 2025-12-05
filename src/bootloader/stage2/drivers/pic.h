#pragma once
#include "../stdint.h"

void PIC_config(u8 offset_pic1, u8 offset_pic2);
void PIC_mask(int irq);
void PIC_unmask(int irq);
void PIC_send_eoi(int irq);
void PIC_disable();

u16 PIC_read_isr();
u16 PIC_read_irr();