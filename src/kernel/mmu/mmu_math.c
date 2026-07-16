#include <stdint.h>

u32 mmu_ceillog2(u64 x)
{
    return x <= 1 ? 0 : 64 - __builtin_clzll(x-1);
}

u32 mmu_floorlog2(u64 x)
{
    return x ? 63 - __builtin_clzll(x) : 0;
}