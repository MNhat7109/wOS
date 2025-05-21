#include "i8259.h"
#include "../../io/io.h"

#define PIC1_CMD_PORT 0x20
#define PIC2_CMD_PORT 0xA0
#define PIC1_DATA_PORT (PIC1_CMD_PORT+1)
#define PIC2_DATA_PORT (PIC2_CMD_PORT+1)

#define PIC_ICW1_ICW4 0x01
#define PIC_ICW1_SNGL 0x02
#define PIC_ICW1_ITR4 0x04
#define PIC_ICW1_LVLL 0x08
#define PIC_ICW1_INIT 0x10

#define PIC_ICW4_8086 0x01
#define PIC_ICW4_AUTO 0x02
#define PIC_ICW4_BUS_MASTER 0x04
#define PIC_ICW4_BUS_SLAVE  0x00
#define PIC_ICW4_BUFFERRED  0x08
#define PIC_ICW4_SFNM 0x10

#define PIC_CMD_EOI 0x20
#define PIC_CMD_READ_IRR 0x0A
#define PIC_CMD_READ_ISR 0x0B

u16 current_mask = 0xFFFF;

void i8259_set_mask(u16 mask)
{
    current_mask = mask;
    outb(PIC1_DATA_PORT, current_mask&0xFF);
    iowait();
    outb(PIC2_DATA_PORT, current_mask>>8);
    iowait();
}

u16 i8259_get_mask()
{
    return inb(PIC1_DATA_PORT) | (inb(PIC2_DATA_PORT)<<8);
}

void i8259_config(u8 offset_pic1, u8 offset_pic2)
{
    i8259_set_mask(0xFFFF);
    // ICW1
    outb(PIC1_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    iowait();
    outb(PIC2_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
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
    outb(PIC1_DATA_PORT, PIC_ICW4_8086);
    iowait();
    outb(PIC2_DATA_PORT, PIC_ICW4_8086);
    iowait();

    outb(PIC1_DATA_PORT, 0);
    iowait();
    outb(PIC2_DATA_PORT, 0);
    iowait();

    i8259_set_mask(0xFFFF);
}

void i8259_mask(int irq)
{
    i8259_set_mask(current_mask | (1<<irq));
}

void i8259_unmask(int irq)
{
    i8259_set_mask(current_mask & ~(1<<irq));
}

void i8259_send_eoi(int irq)
{
    if (irq >= 8)
        outb(PIC2_CMD_PORT, PIC_CMD_EOI);
    outb(PIC1_CMD_PORT, PIC_CMD_EOI);
}

void i8259_disable()
{
    i8259_set_mask(0xFFFF);
}

u16 i8259_read_isr()
{
    outb(PIC1_CMD_PORT, PIC_CMD_READ_ISR);
    outb(PIC2_CMD_PORT, PIC_CMD_READ_ISR);
    return ((u16)inb(PIC1_DATA_PORT)) | ((u16)inb(PIC2_DATA_PORT));
}

u16 i8259_read_irr()
{
    outb(PIC1_CMD_PORT, PIC_CMD_READ_IRR);
    outb(PIC2_CMD_PORT, PIC_CMD_READ_IRR);
    return ((u16)inb(PIC1_DATA_PORT)) | ((u16)inb(PIC2_DATA_PORT));
}

bool i8259_probe()
{
    i8259_disable();
    i8259_set_mask(0x1337);
    return i8259_get_mask() == 0x1337;
}

pic_driver_t i8259_driver;

const pic_driver_t* i8259_get_driver()
{
    i8259_driver.name = "8259 PIC";
    i8259_driver.config = &i8259_config;
    i8259_driver.probe = &i8259_probe;
    i8259_driver.mask = &i8259_mask;
    i8259_driver.unmask = &i8259_unmask;
    i8259_driver.send_eoi = &i8259_send_eoi;
    i8259_driver.disable = &i8259_disable;
    i8259_driver.read_irr = &i8259_read_irr;
    i8259_driver.read_isr = &i8259_read_isr;
    return &i8259_driver;
}
