#include <libk/bitmanip/bitmanip.h>
#include <libk/stdint.h>

#ifdef __x86_64__

usize ctz(usize num)
{
    return __builtin_ctzll(num);
}
usize clz(usize num)
{
    return __builtin_clzll(num);
}

#endif
