#pragma once
#include <stdint.h>

void* memset(void* s, int c, usize n);
int memcmp(const void* s1, const void* s2, usize n);
void* memcpy(void* dest, const void* src, usize n);
void* memmove(void* dest, const void* src, usize n);

usize strlen(const char* s);
const char* strchr(const char* s, char c);