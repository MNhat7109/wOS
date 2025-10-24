#include <libk/mmu/mmu_map.h>
#include <libk/containers/stack.h>
#include <libk/string.h>
#include <libk/stdio.h>

static struct
{
} mmu_map_data;

static const char* const mem_types[] = {
    "<Invalid type>",
    "Free",
    "Reserved",
    "ACPI-reclaimable",
    "ACPI non-volatile",
    "Bad",
    "System reserved",
    "Hole",
    "Unknown",
};