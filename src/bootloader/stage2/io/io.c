#include "io.h"

// u8 inb(u16 port)
// {
//     u8 value;
//     __asm__ volatile ("in %1, %0" : : "a"(value), "Nd"(port));
//     return value;
// }
// void outb(u16 port, u8 value)
// {
//     __asm__ volatile ("out %0, %1" : : "a"(value), "Nd"(port));
// }


void iowait()
{
    outb(0x80, 0);
}