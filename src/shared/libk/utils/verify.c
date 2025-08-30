#include <libk/utils/verify.h>

bool verify_8bit_checksum(void* ptr, usize len)
{
    u8 sum = 0;
    u8* calc_ptr = (u8*)ptr;
    for (usize i=0;i<len;i++)
        sum+=calc_ptr[i];
    return sum == 0;
}