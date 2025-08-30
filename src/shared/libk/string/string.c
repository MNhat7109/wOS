#include <libk/string.h>

void* memset(void* ptr, int val, usize n)
{
    u8* blk = (u8*)ptr;

    for (usize i =0; i<n;i++)
    {
        blk[i] = (u8)val;
    }

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, usize n)
{
    const u8* p1blk = (const u8*)ptr1;
    const u8* p2blk = (const u8*)ptr2;

    for (usize i =0; i<n;i++)
    {
        if (p1blk[i] > p2blk[i]) return 1;
        else if (p1blk[i] < p2blk[i]) return -1;
    }
    return 0;
}

void* memcpy(void* dst, const void* src, usize n)
{
    const u8* srcblk = (const u8*)src;
    u8* dstblk = (u8*)dst;

    for (usize i = 0; i<n; i++)
    {
        dstblk[i] = srcblk[i];
    }
    return dst;
}

void* memmove(void* dst, const void* src, usize n)
{
    u8 * ucdst = (u8 *)dst;

    const u8 * ucsrc = (const u8 *)src;

    if (ucsrc==ucdst || n==0) return dst;
    if (ucdst > ucsrc && ucdst-ucsrc < (int)n)
    {
        int i;
        for (i=n-1;i>=0;i--)
            ucdst[i]=ucsrc[i];
        return dst;
    }
    if (ucsrc > ucdst && ucsrc-ucdst < (int)n)
    {
        unsigned long i;
        for (i=0;i<n;i++)
            ucdst[i]=ucsrc[i];
        return dst;
    }
    memcpy(dst,src, n);
    return dst;
}

const char* strchr(const char* str, char ch)
{
    if (!str) return NULL;
    while (*str)
    {
        if (*str == ch) return str;
        str++;
    }
    return NULL;
}

usize strlen(const char* str)
{
    usize i=0;
    while (*str++)
    {
        i++;
    }
    return i;
}

char* strcpy(char* dst, const char* src)
{
    char* orig_dst = dst;
    if (!dst) return NULL;
    if (!src)
    {
        *dst='\0';
        return dst;
    }

    while (*src)
    {
        *dst=*src;
        src++;
        dst++;
    }
    *dst='\0';
    return orig_dst;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1==*s2))
    {
        s1++; s2++;
    }
    return *(u8*)s1 - *(u8*)s2;
}