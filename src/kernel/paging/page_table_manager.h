#pragma once
#include "../stdint.h"
#include <stdbool.h>

void page_manager_init(u32 address);
void page_manager_map_memory(u32 virtual_address, u32 physical_address);
bool page_manager_unmap_memory(u32 virtual_address);