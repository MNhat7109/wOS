#include "pic.h"
#include "../io/io.h"
#include "../stdio.h"

#define PIC1_CMD_PORT 0x20
#define PIC2_CMD_PORT 0xA0
#define PIC1_DATA_PORT (PIC1_CMD_PORT+1)
#define PIC2_DATA_PORT (PIC2_CMD_PORT+1)

enum PIC_ICW1
{
    ICW1_ICW4 = 0x01,
    ICW1_SNGL = 0x02,
    ICW1_INTERVAL4 = 0x04,
    ICW1_LEVEL = 0x08,
    ICW1_INIT = 0x10,
};

enum PIC_ICW4
{
    ICW4_8086 = 0x01,
    ICW4_AUTO = 0x02,
    ICW4_BUF_MASTER = 0x04,
    ICW4_BUF_SLAVE = 0x00,
    ICW4_BUFFERED = 0x08,
    ICW4_SFNM = 0x10,
};

enum PIC_CMD
{
    CMD_EOI = 0x20,
    CMD_READ_IRR = 0x0A,
    CMD_READ_ISR = 0x0B
};


void PIC_config(u8 offset_pic1, u8 offset_pic2)
{
    // ICW1
    outb(PIC1_CMD_PORT, ICW1_INIT | ICW1_ICW4);
    iowait();
    outb(PIC2_CMD_PORT, ICW1_INIT | ICW1_ICW4);
    iowait();
    // ICW2
    outb(PIC1_DATA_PORT, offset_pic1);
    iowait();
    outb(PIC2_DATA_PORT, offset_pic2);
    iowait();
    // ICW3
    outb(PIC1_DATA_PORT, 0b00000100);
    iowait();
    outb(PIC2_DATA_PORT, 0b00000010);
    iowait();
    // ICW4
    outb(PIC1_DATA_PORT, ICW4_8086);
    iowait();
    outb(PIC2_DATA_PORT, ICW4_8086);
    iowait();

    outb(PIC1_DATA_PORT, 0);
    iowait();
    outb(PIC2_DATA_PORT, 0);
    iowait();

}

void PIC_mask(int irq)
{
    u16 port = irq<8?PIC1_DATA_PORT:PIC2_DATA_PORT;
    u8 value = inb(port) | (1<<(irq%8));
    outb(port, value);
}

void PIC_unmask(int irq)
{
    u16 port = irq<8?PIC1_DATA_PORT:PIC2_DATA_PORT;
    u8 value = inb(port) & ~(1<<(irq%8));
    outb(port, value);
}

void PIC_send_eoi(int irq)
{
    if (irq >= 8)
        outb(PIC2_CMD_PORT, CMD_EOI);
    outb(PIC1_CMD_PORT, CMD_EOI);
}

void PIC_disable()
{
    outb(PIC1_CMD_PORT, 0xff);
    outb(PIC2_CMD_PORT, 0xff);
}

u16 PIC_read_isr()
{
    outb(PIC1_CMD_PORT, CMD_READ_ISR);
    outb(PIC2_CMD_PORT, CMD_READ_ISR);
    return ((u16)inb(PIC1_DATA_PORT)) | ((u16)inb(PIC2_DATA_PORT));
}

u16 PIC_read_irr()
{
    outb(PIC1_CMD_PORT, CMD_READ_IRR);
    outb(PIC2_CMD_PORT, CMD_READ_IRR);
    return ((u16)inb(PIC1_DATA_PORT)) | ((u16)inb(PIC2_DATA_PORT));
}