#pragma once

typedef enum irq_types_t
{
    SRC_IOAPIC_GSI,
    SRC_IOAPIC_IRQ,
    SRC_PIC_IRQ,
    SRC_PCI_MSI,
} irq_types_t;
