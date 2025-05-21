#include "../stdio.h"
#include "../video/video.h"

#define STDIO_DEFAULT_FG_COLOR 0xFFFFFF
#define STDIO_DEFAULT_BG_COLOR 0

#define putch(_xp,_yp,_c,_f,_b) video_putch((_xp), (_yp), (_c), (_f), (_b))
#define scroll(_l) video_scroll((_l))

void kputc(char ch)
{
    switch (ch)
    {
        case '\0': break;
        case '\t':
            cx+=4;
            break;
        case '\n':
            cy++;
        case '\r':
            cx=0;
            break;
        default:
            putch(cx, cy, ch, STDIO_DEFAULT_FG_COLOR, STDIO_DEFAULT_BG_COLOR);
            cx++;
            break;
    }
    if (cx > sw)
    {
        cx=0;
        cy++;
    }
    if (cy > sh) 
    {
        scroll(1);
        cy--;
    }

}
void kputs(const char* str)
{
    while (*str)
    {
        kputc(*str);
        str++;
    }
}