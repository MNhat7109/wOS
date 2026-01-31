#pragma once
#include <stdint.h>

typedef struct console_t
{
    int current_state;
    u32 xpos, ypos;
    void* backend;
} console_t;

void console_putchar(char ch);
void console_puts(const char* str);