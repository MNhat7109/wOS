#pragma once
#include "../../stdint.h"

struct hpet_driver_t;
void hpet_write(struct hpet_driver_t* self, u32 offset, u64 value);
u64 hpet_read(struct hpet_driver_t* self, u32 offset);
void hpet_write32(struct hpet_driver_t* self, u32 offset, u32 value);
u32 hpet_read32(struct hpet_driver_t* self, u32 offset);
void hpet_read_capabilities(struct hpet_driver_t* self);