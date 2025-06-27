#include "i8259.h"
#include "../../io/io.h"
#include "../driver.h"
#include "../../stdio.h"

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

generic_driver_io_t pic_io_util_pack = 
{
    .pool_value = 0,
    .cmd_sig = PIC_DRV_CMD_IDLE,
    .send = false,
    .receive = false
};
u16 current_mask = 0xFFFF;
u8 offset_pic1=0, offset_pic2=0;

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

void i8259_config()
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

u32 i8259_read()
{
    if (!pic_io_util_pack.receive) return 0;
    pic_io_util_pack.pool_value = 0;
    pic_io_util_pack.receive=false;
    u32 ret_val = 0;
    switch (pic_io_util_pack.cmd_sig)
    {
    case PIC_DRV_CMD_RECEIVE_IRR:
        ret_val = i8259_read_irr();
        break;
    case PIC_DRV_CMD_RECEIVE_ISR:
        ret_val = i8259_read_isr();
        break;
    default:
        break;
    }
    pic_io_util_pack.cmd_sig = PIC_DRV_CMD_IDLE;
    return ret_val;
}

void i8259_write(int pool, u32 value)
{
    pic_io_util_pack.pool_value = value;
    switch (pool)
    {
    case DRIVER_CMD:
        switch (pic_io_util_pack.pool_value)
        {
        case PIC_DRV_CMD_SEND_EOI:
        case PIC_DRV_CMD_SEND_MASK_VAL:
        case PIC_DRV_CMD_SEND_MASK:
        case PIC_DRV_CMD_SEND_UNMASK:
        case PIC_DRV_CMD_SEND_CDW1:
        case PIC_DRV_CMD_SEND_CDW2:
            pic_io_util_pack.cmd_sig = pic_io_util_pack.pool_value;
            pic_io_util_pack.pool_value = 0;
            pic_io_util_pack.send=true;
            break;
        case PIC_DRV_CMD_RECEIVE_IRR:
        case PIC_DRV_CMD_RECEIVE_ISR:
            pic_io_util_pack.cmd_sig = pic_io_util_pack.pool_value;
            pic_io_util_pack.pool_value = 0;
            pic_io_util_pack.receive = true;
            break;
        }
        break;
    default:
        pic_io_util_pack.send=false;
        switch (pic_io_util_pack.cmd_sig)
        {
            case PIC_DRV_CMD_SEND_EOI:
            i8259_send_eoi(pic_io_util_pack.pool_value);
            pic_io_util_pack.pool_value = 0;
            break;
            case PIC_DRV_CMD_SEND_MASK:
            i8259_mask(pic_io_util_pack.pool_value);
            pic_io_util_pack.pool_value = 0;
            break;
            case PIC_DRV_CMD_SEND_UNMASK:
            i8259_unmask(pic_io_util_pack.pool_value);
            pic_io_util_pack.pool_value = 0;
            case PIC_DRV_CMD_SEND_MASK_VAL:
            i8259_set_mask(pic_io_util_pack.pool_value);
            pic_io_util_pack.pool_value = 0;
            break;
        case PIC_DRV_CMD_SEND_CDW1:
            offset_pic1 = pic_io_util_pack.pool_value;
            pic_io_util_pack.pool_value = 0;
            break;
        case PIC_DRV_CMD_SEND_CDW2:
            offset_pic2 = pic_io_util_pack.pool_value;
            pic_io_util_pack.pool_value = 0;
            break;
        default:
            break;
        }
        pic_io_util_pack.cmd_sig = PIC_DRV_CMD_IDLE;
        break;
    }
}

generic_driver_t i8259_driver = {
    .name = "8259 PIC",
    .probe = &i8259_probe,
    .read = &i8259_read,
    .write = &i8259_write,
    .config = &i8259_config,
    .disable = &i8259_disable
};

const generic_driver_t* i8259_get_driver()
{
    return &i8259_driver;
}