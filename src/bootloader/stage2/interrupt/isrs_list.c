#include "idt.h"

#define DEFINE_ISR(n) __attribute__((cdecl)) void _x86_ISR##n(void);

DEFINE_ISR(0)
DEFINE_ISR(1)
DEFINE_ISR(2)
DEFINE_ISR(3)
DEFINE_ISR(4)
DEFINE_ISR(5)
DEFINE_ISR(6)
DEFINE_ISR(7)
DEFINE_ISR(8)
DEFINE_ISR(9)
DEFINE_ISR(10)
DEFINE_ISR(11)
DEFINE_ISR(12)
DEFINE_ISR(13)
DEFINE_ISR(14)
DEFINE_ISR(15)
DEFINE_ISR(16)
DEFINE_ISR(17)
DEFINE_ISR(18)
DEFINE_ISR(19)
DEFINE_ISR(20)
DEFINE_ISR(21)
DEFINE_ISR(22)
DEFINE_ISR(23)
DEFINE_ISR(24)
DEFINE_ISR(25)
DEFINE_ISR(26)
DEFINE_ISR(27)
DEFINE_ISR(28)
DEFINE_ISR(29)
DEFINE_ISR(30)
DEFINE_ISR(31)
DEFINE_ISR(32)
DEFINE_ISR(33)
DEFINE_ISR(34)
DEFINE_ISR(35)
DEFINE_ISR(36)
DEFINE_ISR(37)
DEFINE_ISR(38)
DEFINE_ISR(39)
DEFINE_ISR(40)
DEFINE_ISR(41)
DEFINE_ISR(42)
DEFINE_ISR(43)
DEFINE_ISR(44)
DEFINE_ISR(45)
DEFINE_ISR(46)
DEFINE_ISR(47)

void ISR_init_gates(void)
{
    IDT_init_entry(0, _x86_ISR0, 0x08, 0xE);
    IDT_init_entry(1, _x86_ISR1, 0x08, 0xE);
    IDT_init_entry(2, _x86_ISR2, 0x08, 0xE);
    IDT_init_entry(3, _x86_ISR3, 0x08, 0xE);
    IDT_init_entry(4, _x86_ISR4, 0x08, 0xE);
    IDT_init_entry(5, _x86_ISR5, 0x08, 0xE);
    IDT_init_entry(6, _x86_ISR6, 0x08, 0xE);
    IDT_init_entry(7, _x86_ISR7, 0x08, 0xE);
    IDT_init_entry(8, _x86_ISR8, 0x08, 0xE);
    IDT_init_entry(9, _x86_ISR9, 0x08, 0xE);
    IDT_init_entry(10, _x86_ISR10, 0x08, 0xE);
    IDT_init_entry(11, _x86_ISR11, 0x08, 0xE);
    IDT_init_entry(12, _x86_ISR12, 0x08, 0xE);
    IDT_init_entry(13, _x86_ISR13, 0x08, 0xE);
    IDT_init_entry(14, _x86_ISR14, 0x08, 0xE);
    IDT_init_entry(15, _x86_ISR15, 0x08, 0xE);
    IDT_init_entry(16, _x86_ISR16, 0x08, 0xE);
    IDT_init_entry(17, _x86_ISR17, 0x08, 0xE);
    IDT_init_entry(18, _x86_ISR18, 0x08, 0xE);
    IDT_init_entry(19, _x86_ISR19, 0x08, 0xE);
    IDT_init_entry(20, _x86_ISR20, 0x08, 0xE);
    IDT_init_entry(21, _x86_ISR21, 0x08, 0xE);
    IDT_init_entry(22, _x86_ISR22, 0x08, 0xE);
    IDT_init_entry(23, _x86_ISR23, 0x08, 0xE);
    IDT_init_entry(24, _x86_ISR24, 0x08, 0xE);
    IDT_init_entry(25, _x86_ISR25, 0x08, 0xE);
    IDT_init_entry(26, _x86_ISR26, 0x08, 0xE);
    IDT_init_entry(27, _x86_ISR27, 0x08, 0xE);
    IDT_init_entry(28, _x86_ISR28, 0x08, 0xE);
    IDT_init_entry(29, _x86_ISR29, 0x08, 0xE);
    IDT_init_entry(30, _x86_ISR30, 0x08, 0xE);
    IDT_init_entry(31, _x86_ISR31, 0x08, 0xE);
    IDT_init_entry(32, _x86_ISR32, 0x08, 0xE);
    IDT_init_entry(33, _x86_ISR33, 0x08, 0xE);
    IDT_init_entry(34, _x86_ISR34, 0x08, 0xE);
    IDT_init_entry(35, _x86_ISR35, 0x08, 0xE);
    IDT_init_entry(36, _x86_ISR36, 0x08, 0xE);
    IDT_init_entry(37, _x86_ISR37, 0x08, 0xE);
    IDT_init_entry(38, _x86_ISR38, 0x08, 0xE);
    IDT_init_entry(39, _x86_ISR39, 0x08, 0xE);
    IDT_init_entry(40, _x86_ISR40, 0x08, 0xE);
    IDT_init_entry(41, _x86_ISR41, 0x08, 0xE);
    IDT_init_entry(42, _x86_ISR42, 0x08, 0xE);
    IDT_init_entry(43, _x86_ISR43, 0x08, 0xE);
    IDT_init_entry(44, _x86_ISR44, 0x08, 0xE);
    IDT_init_entry(45, _x86_ISR45, 0x08, 0xE);
    IDT_init_entry(46, _x86_ISR46, 0x08, 0xE);
    IDT_init_entry(47, _x86_ISR47, 0x08, 0xE);
}