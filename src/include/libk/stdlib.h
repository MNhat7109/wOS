#pragma once
#include <libk/stdint.h>

void* kmalloc(usize size);
void* kcalloc(usize size);
void* krealloc(void* ptr, usize size);
void kfree(void* ptr);