#pragma once
#include <libk/stdint.h>
#include <devices/driver.h>
#include <devices/pio.h>

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

struct pic_driver_t
{
    struct generic_driver_t driver_hdr;
    pio_layer_t pio_utils;
    u8 offset_pic1, offset_pic2;
    u16 __current_mask;
    void (*mask)(struct pic_driver_t*, int);
    void (*unmask)(struct pic_driver_t*, int);
    void (*send_eoi)(struct pic_driver_t*, int);
    u16 (*read_isr)(struct pic_driver_t*);
    u16 (*read_irr)(struct pic_driver_t*);
    u16 (*read_imr)(struct pic_driver_t*);
} __attribute__((packed));

const struct generic_driver_t* i8259_get_driver();