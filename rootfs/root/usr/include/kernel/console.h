#pragma once
#include <stdint.h>

#define CONSOLE_MAX_ROWS 1024
#define CONSOLE_MAX_COLS 256

typedef enum
{
    CON_COLOR_BLACK = 0,
    CON_COLOR_BLUE,
    CON_COLOR_GREEN,
    CON_COLOR_CYAN,
    CON_COLOR_RED,
    CON_COLOR_PURPLE,
    CON_COLOR_BROWN,
    CON_COLOR_GRAY,
    CON_COLOR_DARK_GRAY,
    CON_COLOR_LIGHT_BLUE,
    CON_COLOR_LIGHT_GREEN,
    CON_COLOR_LIGHT_CYAN,
    CON_COLOR_LIGHT_RED,
    CON_COLOR_LIGHT_PURPLE,
    CON_COLOR_YELLOW,
    CON_COLOR_WHITE,
} console_color_t;

typedef enum 
{
    CON_STATE_NORMAL,
    CON_STATE_ESC,
    CON_STATE_CSI,
} console_state_t;

typedef struct console_backend_t
{
    void (*putchar)(char ch);
    void (*set_cursor)(u32 col, u32 row);
    void (*clear)();
    void (*set_color)(u32 fg, u32 bg);
} console_backend_t;

typedef struct console_line_t
{
    char cols[CONSOLE_MAX_COLS];
    u16 len;
} console_line_t;

typedef struct console_t console_t;

typedef struct console_t
{
    u32 number;
    console_line_t lines[CONSOLE_MAX_ROWS];

    int current_state;
    u32 cols, rows;
    u32 fg, bg;
    u32 tab_size;
    console_backend_t* backend;

    void (*writechar)(console_t* console, char ch);
    void (*writestr)(console_t* console, const char* str);
    void (*scroll)(console_t* console, int lines);
    void (*clear)(console_t* console);
} console_t;

void console_init_dev(console_t* con, u32 number, u32 cols, u32 rows, console_backend_t* backend);