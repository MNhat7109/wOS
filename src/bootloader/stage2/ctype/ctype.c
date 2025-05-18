#include "ctype.h"

char toupper(char ch)
{
    return ch>='a'&&ch<='z'?ch-' ':ch;
}