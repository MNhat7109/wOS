#pragma once
#include <list_defs.h>

#ifdef __cplusplus
extern "C" {
#endif


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