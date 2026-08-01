#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ds_list_t ds_list_t;
typedef struct ds_list_node_t ds_list_node_t;

typedef struct ds_list_node_t
{
    ds_list_t* list;
    ds_list_node_t* prev;
    ds_list_node_t* next;
} ds_list_node_t;

typedef struct ds_list_t
{
    ds_list_node_t* head;
    ds_list_node_t* tail;
} ds_list_t;

#define container_of(_p, _t, _n) ((_t*)(((uint8_t*)(_p)-offsetof(_t,_n))))

void ds_list_init(ds_list_t* list);
void ds_list_node_init(ds_list_node_t* node);

int ds_list_push_front(ds_list_t* list, ds_list_node_t* node);
int ds_list_push_back(ds_list_t* list, ds_list_node_t* node);

int ds_list_insert_back_of(ds_list_t* list, ds_list_node_t* pos, ds_list_node_t* node);
int ds_list_insert_front_of(ds_list_t* list, ds_list_node_t* pos, ds_list_node_t* node);

int ds_list_remove(ds_list_t* list, ds_list_node_t* node);

#ifdef __cplusplus
}
#endif