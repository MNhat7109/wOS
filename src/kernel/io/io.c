#include "io.h"

void iowait()
{
    outb(0x80, 0);
}