#include <libk/stdio.h>
#include <libk/video/video.h>

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
            cx = (cx + 4) & ~(4 - 1); // align to next multiple of 4
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
    if (cx >= char_sw)
    {
        cx=0;
        cy++;
    }
    if (cy >= char_sh) 
    {
        scroll(1);
        cy--;
        cx=0;
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

void ksnputc(stdio_ctx_t* ctx, char ch)
{
    if (ctx->pos < ctx->len)
        ctx->buf[ctx->pos] = ch;

    ctx->pos++;
    ctx->cnt++;
}

void ksnputs(stdio_ctx_t* ctx, const char* str)
{
    while (*str)
    {
        ksnputs(ctx, *str);
        str++;
    }
}

void kclrscr()
{
    video_clear(STDIO_DEFAULT_BG_COLOR);
}