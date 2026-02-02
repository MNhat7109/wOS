#include <kernel/console.h>

void console_writechar(console_t* console, char ch);
void console_writestr(console_t* console, const char* str);
void console_scroll(console_t* console, int lines);
void console_clear(console_t* console);

void console_init_dev(console_t* con, u32 number, u32 cols, u32 rows, console_backend_t* backend)
{
    if (!con) return;

    con->number = number;
    con->current_state = CON_STATE_NORMAL;
    con->cols = cols;
    con->rows = rows;
    con->tab_size = 8;
    con->backend = backend;

    con->clear = &console_clear;
    con->scroll = &console_scroll;
    con->writechar = &console_writechar;
    con->writestr = &console_writestr;
}