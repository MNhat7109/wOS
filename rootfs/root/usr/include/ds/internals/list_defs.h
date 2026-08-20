#pragma once
#include <ds_internals.h>

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