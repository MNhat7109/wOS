#pragma once

#include <bst_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

void ds_binary_search_tree_init(
    ds_binary_search_tree_t* tree, 
    void* ctx, 
    ds_bst_comp_t cmp,
    int duplicate_handling
);
void ds_binary_search_tree_node_init(ds_binary_search_tree_node_t* node);

ds_binary_search_tree_node_t* 
ds_binary_search_tree_successor_of(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node
);

ds_binary_search_tree_node_t* 
ds_binary_search_tree_predecessor_of(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node
);

int ds_binary_search_tree_attach(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node); 
int ds_binary_search_tree_remove(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node, ds_bst_remove_info_t* addl_info); 
ds_binary_search_tree_node_t* ds_binary_search_tree_search(ds_binary_search_tree_t* tree,
    ds_binary_search_tree_node_t* node);

#ifdef __cplusplus
}
#endif