#include <kutils/mmu/utils.h>
#include <libk/bitmanip/bitmanip.h>

usize mmu_klog2(usize num)
{
    return (sizeof(num)*8-1) - clz(num);
}

usize mmu_find_next_po2(usize num)
{
    if (num == 0) return 0;
    return (usize)1 << ((sizeof(num)*8)- clz(num-1));
}

usize mmu_find_prev_po2(usize num)
{
    if (num==0) return 0;
    return (usize)1 << ((sizeof(num)*8-1) - clz(num));
}