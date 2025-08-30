#pragma once
#include <libk/stdint.h>

void swap_n(
    u8* a,
    u8* b,
    usize item_size
);

void sort(
    void* base,
    usize n,
    usize item_size,
    int (*cmp)(const void*, const void*)
);