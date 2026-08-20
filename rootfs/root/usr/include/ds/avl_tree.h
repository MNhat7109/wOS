#pragma once
#include <bst_defs.h>

typedef struct ds_avl_tree_node_t ds_avl_tree_node_t;

typedef struct ds_avl_tree_node_t
{
    ds_binary_search_tree_node_t bst;
    ssize_t height;
} ds_avl_tree_node_t;

typedef ds_binary_search_tree_t ds_avl_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

void ds_avl_tree_init(ds_avl_tree_t* tree, void* ctx, ds_bst_comp_t cmp, int dup_policy);
void ds_avl_tree_node_init(ds_avl_tree_node_t* node);
ds_avl_tree_node_t* ds_avl_tree_convert_raw(ds_binary_search_tree_node_t* node);

ssize_t ds_avl_tree_balance_of(ds_avl_tree_node_t* node);

int ds_avl_tree_attach(ds_avl_tree_t* tree, ds_avl_tree_node_t* node);
int ds_avl_tree_remove(ds_avl_tree_t* tree, ds_avl_tree_node_t* node, ds_bst_remove_info_t* addl_info);
ds_avl_tree_node_t* ds_avl_tree_find(ds_avl_tree_t* tree, ds_avl_tree_node_t* node);

#ifdef __cplusplus
}
#endif