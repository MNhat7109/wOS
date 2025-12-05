#pragma once

typedef enum
{
    CONSOLE_MODE_VIDEO,
    CONSOLE_MODE_SERIAL,
    CONSOLE_MODE_BOTH,
} console_type_t;

typedef struct boot_info_t boot_info_t;

void console_init(int mode, boot_info_t* boot_info);
void console_write(const char* str);
void console_putchar(char ch);
void console_switch_mode(int mode);
int console_get_mode();