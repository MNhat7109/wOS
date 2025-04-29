#include "string.h"

void* memset(void* ptr, int val, u32 n)
{
    u8* blk = (u8*)ptr;

    for (u32 i =0; i<n;i++)
    {
        blk[i] = (u8)val;
    }

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, u32 n)
{
    const u8* p1blk = (const u8*)ptr1;
    const u8* p2blk = (const u8*)ptr2;

    for (u32 i =0; i<n;i++)
    {
        if (p1blk[i] > p2blk[i]) return 1;
        else if (p1blk[i] < p2blk[i]) return -1;
    }
    return 0;
}

void* memcpy(void* dst, const void* src, u32 n)
{
    const u8* srcblk = (const u8*)src;
    u8* dstblk = (u8*)dst;

    for (u32 i = 0; i<n; i++)
    {
        dstblk[i] = srcblk[i];
    }
    return dst;
}

void* memmove(void* dst, const void* src, u32 n)
{
    u32 blknum = n/100, lastblk=n%100;
    u8 temp[100];
    for (u32 i =0 ;i<blknum;i++)
    {
        memcpy(temp, src, 100);
        memcpy(dst, temp, 100);
        memset(temp, 0, 100);
    }
    memcpy(temp, src, lastblk);
    memcpy(dst, temp, lastblk);
    return dst;
}