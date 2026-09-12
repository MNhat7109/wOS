#pragma once
#include <stdint.h>


void ioapic_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id);
void ioapic_cut_gsi(u8 gsi);