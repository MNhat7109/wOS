#pragma once

#include <binary_tree_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

void ds_binary_tree_init(ds_binary_tree_t* binary_tree);
void ds_binary_tree_node_init(ds_binary_tree_node_t* node);

int ds_binary_tree_transplant(ds_binary_tree_t* binary_tree, 
    ds_binary_tree_node_t* node,
    ds_binary_tree_node_t* replacement
);

int ds_binary_tree_side_of(ds_binary_tree_node_t* node);
ds_binary_tree_node_t* ds_binary_tree_rotate_left(ds_binary_tree_t* binary_tree, ds_binary_tree_node_t* pivot);
ds_binary_tree_node_t* ds_binary_tree_rotate_right(ds_binary_tree_t* binary_tree, ds_binary_tree_node_t* pivot);

#ifdef __cplusplus
}
#endif