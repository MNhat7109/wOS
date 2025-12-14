#include "console.h"
#include "../boot_info.h"
#include "serial.h"
#include "video.h"

typedef enum
{
    PUTCH_STATE_NORMAL,
    PUTCH_STATE_ESCAPE,
    PUTCH_STATE_ESCAPE_CSI,
} console_putchar_video_state_t;

typedef enum
{
    PUTCH_ESCAPE_CSI,
    PUTCH_ESCAPE_NONE,
} console_putchar_state_esc_t;

static struct
{
    int current_mode;
    u32 xpos, ypos, screen_char_width, screen_char_height;
    int current_video_state;
    struct
    {
        u8 param_count;
        u32 params[4];
        char cmd;
    } csi;
} console_data;

void console_putchar_serial(char ch);
void console_putchar_video(char ch);
void console_video_handle_csi();

void console_init(int mode, boot_info_t* boot_info)
{
    video_init(boot_info);
    serial_init();

    console_data.xpos=0;
    console_data.ypos=0;
    console_data.screen_char_height=video_get_SCH();
    console_data.screen_char_width=video_get_SCW();

    console_switch_mode(mode);
    console_write("Console: Hello there! If you can see this message, console works!\n");        
}

void console_putchar(char ch)
{
    console_data.current_video_state=PUTCH_STATE_NORMAL;

    switch (console_data.current_mode)
    {
        case CONSOLE_MODE_SERIAL:
            console_putchar_serial(ch); break;
        case CONSOLE_MODE_VIDEO:
            console_putchar_video(ch); break;
        case CONSOLE_MODE_BOTH:
            console_putchar_serial(ch); 
            console_putchar_video(ch); 
            break;
        default: break;
    }
}

void console_write(const char* str)
{
    while (*str)
    {
        console_putchar(*str);
        str++;
    }
}

void console_switch_mode(int mode)
{
    console_data.current_mode = mode;
}

int console_get_mode()
{
    return console_data.current_mode;
}

void console_putchar_serial(char ch)
{
    if (ch == '\n') serial_putch('\r');
    serial_putch(ch);
}

void console_putchar_video(char ch)
{
    // We need a state machine for this.
    switch (console_data.current_video_state)
    {
        case PUTCH_STATE_NORMAL:        
            switch (ch)
            {
                case '\0': break;
                case '\t':
                    do
                    {
                        video_putch(' ', console_data.xpos, console_data.ypos);
                        console_data.xpos++;
                    } while (console_data.xpos % 8 != 0);
                    break;
                case '\n': console_data.ypos++;
                case '\r': console_data.xpos=0; break;
                case '\x1B': 
                    console_data.current_video_state = PUTCH_STATE_ESCAPE;
                    break;
                default:
                    video_putch(ch, console_data.xpos, console_data.ypos);
                    console_data.xpos++;
            }
            break;
        case PUTCH_STATE_ESCAPE:
            switch (ch)
            {
                case '[':
                    console_data.current_video_state = PUTCH_STATE_ESCAPE_CSI;
                    break;
                default: 
                    console_data.current_video_state = PUTCH_STATE_NORMAL;
                    break;
            }
            break;
        case PUTCH_STATE_ESCAPE_CSI:
            if (ch >= '0' && ch <= '9')
            {
                console_data.csi.params[console_data.csi.param_count] =
                console_data.csi.params[console_data.csi.param_count]*10+(ch-'0');
            }
            else if (ch == ';')
            {
                if (console_data.csi.param_count < 4)
                console_data.csi.param_count++;
            }
            else
            {
                console_data.csi.cmd = ch;
                console_video_handle_csi();
                console_data.current_video_state = PUTCH_STATE_NORMAL;
            }
            break;
    }       

    if (console_data.xpos >= console_data.screen_char_width)
    {
        console_data.xpos=0;
        console_data.ypos++;
    }

    if (console_data.ypos >= console_data.screen_char_height)
    {
        video_scroll(1);
        console_data.ypos--;
    }

    video_setcursor(console_data.xpos, console_data.ypos);
}

void console_video_handle_csi()
{
    switch (console_data.csi.cmd)
    {
        case 'J':
            if (console_data.csi.params[0] == 2) video_clrscr();
            break;
        case 'H':
            {
                u32 y = console_data.csi.params[0]?console_data.csi.params[0]-1:0;
                u32 x = console_data.csi.params[1]?console_data.csi.params[1]-1:0;
                video_setcursor(x,y);
            }
            break;
        case 'm':
            {
                u8 bg = 0, fg=0;
                // TODO: Add set_color()
            }
        default:
            break;
    }
}