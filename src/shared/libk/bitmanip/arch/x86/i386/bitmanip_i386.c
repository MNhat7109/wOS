#include <libk/bitmanip/bitmanip.h>
#include <libk/stdint.h>

#ifdef __i386__

usize ctz(usize num)
{
    return __builtin_ctz(num);
}
usize clz(usize num)
{
    return __builtin_clz(num);
}

#endif
