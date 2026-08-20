#pragma once
#include <list_defs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ds_list_node_t ds_queue_node_t;

typedef struct ds_queue_t
{
    size_t size;
    ds_list_t list;
} ds_queue_t;

void ds_queue_init(ds_queue_t* queue);
void ds_queue_node_init(ds_queue_node_t* node);

int ds_queue_push(ds_queue_t* queue, ds_queue_node_t* node);
int ds_queue_pop(ds_queue_t* queue);

ds_queue_node_t* ds_queue_front(ds_queue_t* queue);
ds_queue_node_t* ds_queue_back(ds_queue_t* queue);

#ifdef __cplusplus
}
#endif