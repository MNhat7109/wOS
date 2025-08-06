#include "irq_utils.h"


void irq_load_handler(int interrupt, gsi_handler_t handler, void* ctx)
{
    gsi_handler_table[interrupt].handler = handler;
    gsi_handler_table[interrupt].ctx = ctx;
}

void irq_unload_handler(int interrupt)
{
    gsi_handler_table[interrupt].handler = NULL;
    gsi_handler_table[interrupt].ctx = NULL;
}