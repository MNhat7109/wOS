#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

typedef enum
{
    CPU_IPI_FIXED,
    CPU_IPI_LO_PRI,
    CPU_IPI_SMI,
    CPU_IPI_NMI = 4,
    CPU_IPI_INIT,
    CPU_IPI_SIPI
} x86_cpu_ipi_delivery_t;