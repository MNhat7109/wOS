#pragma once
#include "../stdint.h"

void* memset(void* ptr, int val, u32 n);
int memcmp(const void* ptr1, const void* ptr2, u32 n);
void* memcpy(void* dst, const void* src, u32 n);
void* memmove(void* dst, const void* src, u32 n);

const char* strchr(const char* str, char ch);
u32   strlen(const char* str);