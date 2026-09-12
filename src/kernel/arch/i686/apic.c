#include <kernel/sys/hw_int_defs.h>
#include <kernel/sys/hw_interrupt.h>
#include <kernel/arch/i686/ioapic.h>

void apic_ack(int)
{
    lapic_ack();
}

int apic_register_interrupt(int gsi, hw_int_handler_t handler, void* ctx)
{
    int st;
    int vector = int_vector_alloc(); // TODO: 1
    if (vector < 0) return -1;

    st = int_vector_populate(vector, gsi, handler, ctx); // TODO: 1
    if (st <0) return st;

    return 0;
}

void apic_unregister_interrupt(int gsi)
{
    int vector = int_vector_find(gsi); // TODO: 1
    if (vector < 0) return;
    int_vector_free(vector, gsi);
}

void apic_disable_interrupt(int gsi)
{
    ioapic_cut_gsi(gsi);
}

void apic_enable_interrupt(int gsi)
{
    int vector = int_vector_find(gsi);
    if (vector < 0) return;
    ioapic_redirect_gsi(gsi, vector, lapic_get_core_id());
}

// TODO 1: Needs a header file for interrupt vector allocation