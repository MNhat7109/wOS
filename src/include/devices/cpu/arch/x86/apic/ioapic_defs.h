#pragma once

#define IOAPIC_IOREGSEL 0x00
#define IOAPIC_IOREGWIN 0x10

#define IOAPIC_REGTBLn(n) (0x10+((n)<<1))
#define IOAPIC_REGTBLn_LO(n) (IOAPIC_REGTBLn(n))
#define IOAPIC_REGTBLn_HI(n) (IOAPIC_REGTBLn(n)+1)

#define IOAPIC_REG_ID 0x0
#define IOAPIC_REG_VER 0x1
#define IOAPIC_REG_ARB 0x2
