#pragma once
#include <list_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ds_list_node_t ds_stack_node_t;

typedef struct ds_stack_t
{
    size_t size;
    ds_list_t list;
} ds_stack_t;

void ds_stack_init(ds_stack_t* stack);
void ds_stack_node_init(ds_stack_node_t* node);

int ds_stack_push(ds_stack_t* stack, ds_stack_node_t* node);
int ds_stack_pop(ds_stack_t* stack);

ds_stack_node_t* ds_stack_top(ds_stack_t* stack);

#ifdef __cplusplus
}
#endif