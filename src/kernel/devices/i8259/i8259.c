#include "i8259.h"
#include "../driver.h"

void i8259_pio_wait(struct pic_driver_t* pic_self)
{
    pic_self->pio_utils.writeb(0x80, 0);
}

void i8259_set_mask(struct pic_driver_t* pic_self, u16 mask)
{
    pic_self->__current_mask = mask;
    pic_self->pio_utils.writeb(PIC1_DATA_PORT, pic_self->__current_mask&0xFF);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_DATA_PORT, pic_self->__current_mask>>8);
    i8259_pio_wait(pic_self);
}

u16 i8259_get_mask(struct pic_driver_t* pic_self)
{
    return pic_self->pio_utils.readb(PIC1_DATA_PORT) 
    | (pic_self->pio_utils.readb(PIC2_DATA_PORT)<<8);
}

void i8259_config(struct generic_driver_t* driver)
{
    struct pic_driver_t* pic_self = (struct pic_driver_t*)driver;
    // ICW1
    pic_self->pio_utils.writeb(PIC1_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    i8259_pio_wait(pic_self);
    // ICW2
    pic_self->pio_utils.writeb(PIC1_DATA_PORT, pic_self->offset_pic1);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_DATA_PORT, pic_self->offset_pic2);
    i8259_pio_wait(pic_self);
    // ICW3
    pic_self->pio_utils.writeb(PIC1_DATA_PORT, 0b00000100);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_DATA_PORT, 0b00000010);
    i8259_pio_wait(pic_self);
    // ICW4
    pic_self->pio_utils.writeb(PIC1_DATA_PORT, PIC_ICW4_8086);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_DATA_PORT, PIC_ICW4_8086);
    i8259_pio_wait(pic_self);

    pic_self->pio_utils.writeb(PIC1_DATA_PORT, 0);
    i8259_pio_wait(pic_self);
    pic_self->pio_utils.writeb(PIC2_DATA_PORT, 0);
    i8259_pio_wait(pic_self);

    i8259_set_mask(pic_self, 0xFFFF);
}

void i8259_mask(struct pic_driver_t* pic_self, int irq)
{
    i8259_set_mask(pic_self, pic_self->__current_mask | (1<<irq));
}

void i8259_unmask(struct pic_driver_t* pic_self, int irq)
{
    i8259_set_mask(pic_self, pic_self->__current_mask & ~(1<<irq));
}

void i8259_send_eoi(struct pic_driver_t* pic_self, int irq)
{
    if (irq >= 8)
        pic_self->pio_utils.writeb(PIC2_CMD_PORT, PIC_CMD_EOI);
    pic_self->pio_utils.writeb(PIC1_CMD_PORT, PIC_CMD_EOI);
}

void i8259_disable(struct generic_driver_t* driver)
{
    i8259_set_mask((struct pic_driver_t*)driver,0xFFFF);
}

u16 i8259_read_isr(struct pic_driver_t* pic_self)
{
    pic_self->pio_utils.writeb(PIC1_CMD_PORT, PIC_CMD_READ_ISR);
    pic_self->pio_utils.writeb(PIC2_CMD_PORT, PIC_CMD_READ_ISR);
    return ((u16)pic_self->pio_utils.readb(PIC1_DATA_PORT)) | ((u16)pic_self->pio_utils.readb(PIC2_DATA_PORT));
}

u16 i8259_read_irr(struct pic_driver_t* pic_self)
{
    pic_self->pio_utils.writeb(PIC1_CMD_PORT, PIC_CMD_READ_IRR);
    pic_self->pio_utils.writeb(PIC2_CMD_PORT, PIC_CMD_READ_IRR);
    return ((u16)pic_self->pio_utils.readb(PIC1_DATA_PORT)) | ((u16)pic_self->pio_utils.readb(PIC2_DATA_PORT));
}

bool i8259_probe(struct generic_driver_t* driver)
{
    struct pic_driver_t* pic_self = (struct pic_driver_t*)driver;
    i8259_disable(driver);
    i8259_set_mask(pic_self, 0x1337);
    return i8259_get_mask(pic_self) == 0x1337;
}

struct pic_driver_t i8259_driver = {
    .driver_hdr = {
        .name = "8259 PIC",
        .probe = &i8259_probe,
        .config = &i8259_config,
        .disable = &i8259_disable
    },
    .mask = &i8259_mask,
    .unmask = &i8259_unmask,
    .send_eoi = &i8259_send_eoi,
    .read_isr = &i8259_read_isr,
    .read_irr = &i8259_read_irr,
    .read_imr = &i8259_get_mask
};

const struct generic_driver_t* i8259_get_driver()
{
    i8259_driver.pio_utils = pio_load_defaults();
    return (struct generic_driver_t*)&i8259_driver;
}