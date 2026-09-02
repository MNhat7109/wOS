#include <kernel/sys/hw_int_defs.h>

#define APIC_REMAP_OFFSET 0x30

int apic_init(struct hw_int_data_t* self)
{
    // Jumpstart IOAPIC and LAPIC
    self->init = true;
    return 0;
}

int apic_vector2gsi(struct hw_int_data_t* self, int vector)
{
    if (!self) return -1;
    return vector - APIC_REMAP_OFFSET;
}

void apic_ack(struct hw_int_data_t* self, int gsi)
{
    if (!self) return;
    // TODO
}

void apic_enable_gsi(struct hw_int_data_t* self, int gsi)
{
    if (!self) return;

}

void apic_disable_gsi(struct hw_int_data_t* self, int gsi)
{
    if (!self) return;

}

void apic_enable_all_gsis(struct hw_int_data_t* self)
{
    if (!self) return;

}

void apic_disable_all_gsis(struct hw_int_data_t* self)
{
    if (!self) return;

}

static const hw_int_ops_t ops = {
    .vector2intno = &apic_vector2gsi,
    .init = &apic_init,
    .ack = &apic_ack,
    .disable = &apic_disable_gsi,
    .enable = &apic_enable_gsi,
    .disable_all = &apic_disable_all_gsis,
    .enable_all = &apic_enable_all_gsis
};

const hw_int_ops_t* apic_get_ops()
{
    return &ops;
}