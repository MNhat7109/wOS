#pragma once
#include "../../../stdint.h"

struct ioapic_driver_t;
void ioapic_write(struct ioapic_driver_t* self, u32 base, u8 reg, u32 value);
u32 ioapic_read(struct ioapic_driver_t* self, u32 base, u8 reg);
void ioapic_detect(void* ptr);
void iso_detect(void* ptr);
void ioapic_redirect_gsi(struct ioapic_driver_t* self, u8 gsi, u8 vector, u8 lapic_id);
void ioapic_cut_gsi(struct ioapic_driver_t* self, u8 gsi);
u32 ioapic_irq_to_gsi(struct ioapic_driver_t* self, u8 irq);
