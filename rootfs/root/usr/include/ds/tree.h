#pragma once

#include "internals/tree_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void ds_tree_init(ds_tree_t* tree);
void ds_tree_node_init(ds_tree_node_t* node);

int ds_tree_attach(ds_tree_t* tree, ds_tree_node_t* parent, ds_tree_node_t* node);
int ds_tree_detach(ds_tree_t* tree, ds_tree_node_t* node);
#ifdef __cplusplus
}
#endif