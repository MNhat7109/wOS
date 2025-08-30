#pragma once
#include <libk/stdint.h>

void* memset(void* ptr, int val, usize n);
int memcmp(const void* ptr1, const void* ptr2, usize n);
void* memcpy(void* dst, const void* src, usize n);
void* memmove(void* dst, const void* src, usize n);

const char* strchr(const char* str, char ch);
usize   strlen(const char* str);
int   strcmp(const char* s1, const char* s2);
char* strcpy(char* dst, const char* src);