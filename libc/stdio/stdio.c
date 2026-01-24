#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum printf_STATES
{
    PRINTF_STATE_NORMAL,
    PRINTF_STATE_SPEC,
    PRINTF_STATE_LENGTH,
    PRINTF_STATE_LENGTH_LONG,
    PRINTF_STATE_LENGTH_SHORT,
};

enum printf_STATES_LENGTH
{
    PRINTF_LENGTH_SHORT = 3,
    PRINTF_LENGTH_SHORT_SHORT,
    PRINTF_LENGTH_LONG,
    PRINTF_LENGTH_LONG_LONG,
    PRINTF_LENGTH_NORMAL,
};

const char* digits = "0123456789abcdef";

void printf_unsigned(unsigned long long number, int radix)
{
    char bufferOut[32]; int i=0;
    do
    {
        int idx = number % radix;
        bufferOut[i++] = digits[idx];
        number /= radix;
    } while (number > 0);

    while (--i >= 0) putchar(bufferOut[i]);
}

void printf_signed(long long number, int radix)
{
    if (number < 0) 
    {
        putchar('-');
        printf_unsigned(-number, radix);
    }
    else printf_unsigned(number, radix);
}

void (*putchar)(char ch) = NULL;
void (*puts)(const char* str) = NULL;

void stdio_register_putc(void (*putc_op)(char ch))
{
    putchar = putc_op;
}

void stdio_register_puts(void (*puts_op)(const char* str))
{
    puts = puts_op;
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void vprintf(const char* fmt, va_list args)
{
    int current_state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_NORMAL, radix = 10;
    bool number=false, sign=false;
    
    while (*fmt)
    {
        switch (current_state)
        {
        case PRINTF_STATE_NORMAL:
            switch (*fmt)
            {
            case '%':
                current_state = PRINTF_STATE_LENGTH;
                break;
            default:
                putchar(*fmt);
                break;
            }
            break;
        case PRINTF_STATE_LENGTH:
            switch (*fmt)
            {
            case 'h':
                current_state = PRINTF_STATE_LENGTH_SHORT;
                length = PRINTF_LENGTH_SHORT;
                break;
            case 'l':
                current_state = PRINTF_STATE_LENGTH_LONG;
                length = PRINTF_LENGTH_LONG;
                break;
            default:
                fmt--;
                current_state = PRINTF_STATE_SPEC;
                break;
            }
            break;
        case PRINTF_STATE_LENGTH_LONG:
            switch (*fmt)
            {
            case 'l':
                length = PRINTF_LENGTH_LONG_LONG;
                current_state = PRINTF_STATE_SPEC;
                break;
            default:
                fmt--;
                current_state = PRINTF_STATE_SPEC;
                break;
            }
            break;
        case PRINTF_STATE_LENGTH_SHORT:
            switch (*fmt)
            {
            case 'h':
                length = PRINTF_LENGTH_SHORT_SHORT;
                current_state = PRINTF_STATE_SPEC;
                break;
            default:
                fmt--;
                current_state = PRINTF_STATE_SPEC;
                break;
            }
            break;
        case PRINTF_STATE_SPEC:
            switch (*fmt)
            {
            case '%':
                putchar('%');
                break;
            case 'c':
                putchar((char)va_arg(args, int));
                break;
            case 's':
                puts(va_arg(args, const char*));
                break;
            case 'X':
            case 'p':
            case 'x':
                sign=false; number=true; radix=16;
                break;
            case 'i':
            case 'd':
                sign=true; number=true; radix=10;
                break;
            case 'u':
                sign=false; number=true; radix=10;
                break;
            case 'o':
                sign=false; number=true; radix=8;
                break;
            default: break;
            }

            if (number)
            {
                switch (length)
                {
                case PRINTF_LENGTH_SHORT:
                case PRINTF_LENGTH_SHORT_SHORT:
                case PRINTF_LENGTH_NORMAL:
                    if (sign) 
                        printf_signed(va_arg(args,  int), radix);
                    else printf_unsigned(va_arg(args, unsigned int), radix);
                    break;
                case PRINTF_LENGTH_LONG:
                    if (sign) 
                        printf_signed(va_arg(args, long), radix);
                    else printf_unsigned(va_arg(args, unsigned long), radix);
                    break;
                case PRINTF_LENGTH_LONG_LONG:
                    if (sign) 
                        printf_signed(va_arg(args, long long), radix);
                    else printf_unsigned(va_arg(args, unsigned long long), radix);
                    break;
                }
            }
            length = PRINTF_LENGTH_NORMAL;
            current_state = PRINTF_STATE_NORMAL;
            number=false; sign=false;
            radix =10;
            break;
        }
        fmt++;
    }
}