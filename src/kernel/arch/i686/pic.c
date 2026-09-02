#include <kernel/sys/hw_int_defs.h>
#include <kernel/arch/i686/io.h>
#include <stdbool.h>

#define PIC_REMAP_OFFSET 0x20 /* This should allow us to map IRQs from 0x20-0x2F*/

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

#define io_wait() outb(0x80, 0);

static struct
{
    u16 current_mask;
} pic_data;

void pic_set_mask(u16 mask)
{
    pic_data.current_mask = mask;

    outb(PIC1_DATA_PORT, pic_data.current_mask & 0xFF); // PIC1 -> Low 8
    io_wait();
    outb(PIC2_DATA_PORT, pic_data.current_mask >> 8); // PIC2 -> High 8
    io_wait();
}

u16 pic_get_mask()
{
    return (inb(PIC2_DATA_PORT) << 8) | (inb(PIC1_DATA_PORT));
}

bool pic_existence_check()
{
    // The idea is to set the mask of the PIC to an arbitrary number, then retrieve that mask back and compare it to the number.
    pic_set_mask(0xFFFF);

    pic_set_mask(0xB00B);
    return (pic_get_mask() != 0xB00B);
}

int pic_init(struct hw_int_data_t* self)
{
    if (!self) return -1;

    // Probe the PIC first!
    if (!pic_existence_check()) return -1;

    // Remap!
    outb(PIC1_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    // Map PIC1 at offset 0x20, and PIC2 at 0x28.
    // As each PIC holds 8 IRQs, doing so will remap all 15 (+1 cascade) available IRQs.
    outb(PIC1_DATA_PORT, PIC_REMAP_OFFSET);
    io_wait();
    outb(PIC2_DATA_PORT, PIC_REMAP_OFFSET+8);
    io_wait();

    // Because we use both PICs, one of them must be a master, and the other must be a slave
    // Describing this relationship can be possible on ICW3

    // PIC1 is Master
    outb(PIC1_DATA_PORT, (1 << 2)); // There's a slave PIC at IRQ2 (hence the +1 cascade!)
    // PIC2 is Slave
    outb(PIC2_DATA_PORT, 2); // PIC2, you reside at IRQ2
    
    // ICW4
    outb(PIC1_DATA_PORT, PIC_ICW4_8086);
    io_wait();
    outb(PIC2_DATA_PORT, PIC_ICW4_8086);
    io_wait();

    // Disable (Mask) all interrrupts by default, and only enable (Unmask) it when prompted
    pic_set_mask(0xFFFF);

    // Set up vars
    self->init = true;
    self->handler_cnt = 16;
    return 0;
}

int pic_vector2irq(struct hw_int_data_t* self, int vector)
{
    if (!self) return -1;
    return vector - PIC_REMAP_OFFSET;
}

void pic_ack(struct hw_int_data_t* self, int irq)
{
    if (!self) return;
    outb((irq >= 8 ? PIC2_CMD_PORT : PIC1_CMD_PORT), PIC_CMD_EOI);
}

void pic_enable_irq(struct hw_int_data_t* self, int irq)
{
    if (!self) return;
    pic_set_mask(pic_data.current_mask | (1<<irq));
}

void pic_disable_irq(struct hw_int_data_t* self, int irq)
{
    if (!self) return;
    pic_set_mask(pic_data.current_mask & ~(1<<irq));
}

void pic_enable_all_irqs(struct hw_int_data_t* self)
{
    if (!self) return;
    pic_set_mask(0);
}

void pic_disable_all_irqs(struct hw_int_data_t* self)
{
    if (!self) return;
    pic_set_mask(0xFFFF);
}

static const hw_int_ops_t ops = {
    .vector2intno = &pic_vector2irq,
    .init = &pic_init,
    .ack = &pic_ack,
    .disable = &pic_disable_irq,
    .enable = &pic_enable_irq,
    .disable_all = &pic_disable_all_irqs,
    .enable_all = &pic_enable_all_irqs
};

const hw_int_ops_t* pic_get_ops()
{
    return &ops;
}