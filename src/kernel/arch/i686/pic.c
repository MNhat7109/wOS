#include <stdint.h>
#include <stdbool.h>
#include <kernel/arch/i686/io.h>
#include <kernel/arch/i686/irq.h>
#include <kernel/irq.h>
#include <kernel/debug.h>

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

#define io_wait() outb(0x80, 0)
static struct
{
    u8 remap_offset;
    u16 current_mask;
} pic_data;

void pic_register_irq(u8 irq, irq_dispatcher_cb_t cb, void* ctx);
void pic_unregister_irq(u8 irq, irq_dispatcher_cb_t cb);
void pic_ack(u8 irq);

static irq_ops_t pic_irq_ops = (irq_ops_t){
    .reg = &pic_register_irq,
    .unreg = &pic_unregister_irq,
    .ack = &pic_ack
};

static void pic_set_mask(u16 mask)
{
    pic_data.current_mask = mask;

    outb(PIC1_DATA_PORT, pic_data.current_mask & 0xFF); // PIC1 -> Low 8
    io_wait();
    outb(PIC2_DATA_PORT, pic_data.current_mask >> 8); // PIC2 -> High 8
    io_wait();
}

static u16 pic_get_mask()
{
    return (inb(PIC2_DATA_PORT) << 8) | (inb(PIC1_DATA_PORT));
}

void pic_enable_irq(u8 irq)
{
    pic_set_mask(pic_data.current_mask | (1<<irq));
}

void pic_disable_irq(u8 irq)
{
    pic_set_mask(pic_data.current_mask & ~(1<<irq));
}

static bool pic_existence_check()
{
    // The idea is to set the mask of the PIC to an arbitrary number, then retrieve that mask back and compare it to the number.
    pic_set_mask(0xFFFF);

    pic_set_mask(0xB00B);
    return (pic_get_mask() != 0xB00B);
}

int pic_init(u8 PIC_REMAP_OFFSET)
{
    kdebugf(DEBUG_INFO, MODULE_IRQ, "Probing PIC...\n");
    // Probe the PIC first!
    // if (!pic_existence_check()) return -1;
    kdebugf(DEBUG_INFO, MODULE_IRQ, "Setting up PIC...\n");
    
    pic_data.remap_offset = PIC_REMAP_OFFSET;
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

    irq_load_ops(&pic_irq_ops);
    return 0;
}

void pic_register_irq(u8 irq, irq_dispatcher_cb_t cb, void* ctx)
{
    if (irq >= 16) return;
    if (!cb) return;
    u8 vector = pic_data.remap_offset+irq;
    irq_dispatch_blk_t* blk = irq_vector_find(SRC_PIC_IRQ, vector, cb);
    if (blk) return;
    int st = irq_vector_populate(SRC_PIC_IRQ, vector, irq, cb, ctx);
    if (st < 0) return;

    pic_enable_irq(irq);
}

void pic_unregister_irq(u8 irq, irq_dispatcher_cb_t cb)
{
    if (irq >= 16) return;

    u8 vector = pic_data.remap_offset +irq;
    irq_vector_free(SRC_PIC_IRQ, vector, cb);

    pic_disable_irq(irq);
}

void pic_ack(u8 irq)
{
    outb((irq >= 8 ? PIC2_CMD_PORT : PIC1_CMD_PORT), PIC_CMD_EOI);
}
