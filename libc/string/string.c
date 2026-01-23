#include <string.h>

void* memset(void* s, int c, usize n)
{
    for (usize i=0;i<n;i++) ((u8*)s)[i] = (u8)c;
    return s;
}

int memcmp(const void* s1, const void* s2, usize n)
{
    for (usize i=0;i<n;i++)
    {
        u8 cur_s1 = ((const u8*)s1)[i];
        u8 cur_s2 = ((const u8*)s2)[i];
        if (cur_s1 > cur_s2) return 1;
        if (cur_s1 < cur_s2) return -1;
    }
    return 0;
}

void* memcpy(void* dest, const void* src, usize n)
{
    for (usize i=0;i<n;i++) ((u8*)dest)[i] = ((const u8*)src)[i];
    return dest;
}

void* memmove(void* dest, const void* src, usize n)
{
    if (src == dest || n ==0) return dest;
    if (src < dest && (u8*)dest-(const u8*)src < (isize)n)
    {
        for (usize i=n-1;i>=0;i--) ((u8*)dest)[i] = ((const u8*)src)[i];
        goto done;
    }

    memcpy(dest,src, n);
done:
    return dest;
}

usize strlen(const char* s)
{
    usize i=0;
    while (*s++) i++;
    return i;
}

const char* strchr(const char* s, char c)
{
    if (!s) return s;

    while (*s++)
    {
        if (*s == c) return s;
    }
    return NULL;
}