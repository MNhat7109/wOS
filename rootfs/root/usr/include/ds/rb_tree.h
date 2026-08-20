#pragma once
#include <bst_defs.h>

typedef struct ds_rb_tree_node_t ds_rb_tree_node_t;

typedef struct ds_rb_tree_node_t
{
    ds_binary_search_tree_node_t bst;
    int color;
} ds_rb_tree_node_t;

typedef ds_binary_search_tree_t ds_rb_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

void ds_rb_tree_init(ds_rb_tree_t* tree, void* ctx, ds_bst_comp_t cmp, int dup_policy);
void ds_rb_tree_node_init(ds_rb_tree_node_t* node);

ds_rb_tree_node_t* ds_rb_tree_convert_raw(ds_binary_search_tree_node_t* node);
int ds_rb_tree_attach(ds_rb_tree_t* tree, ds_rb_tree_node_t* node);
int ds_rb_tree_remove(ds_rb_tree_t* tree, ds_rb_tree_node_t* node, ds_bst_remove_info_t* addl_info);
ds_rb_tree_node_t* ds_rb_tree_find(ds_rb_tree_t* tree, ds_rb_tree_node_t* node);

#ifdef __cplusplus
}
#endif