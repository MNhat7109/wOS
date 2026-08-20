#pragma once

#include <binary_tree_defs.h>

typedef enum
{
    
    DS_BST_INSERT_DUP_NOT_ALLOWED = 2,
    DS_BST_DELETE_ON_NULL_ROOT,
    DS_BST_DELETE_NOT_FOUND,
} ds_bst_status_t;

typedef ds_binary_tree_node_t ds_binary_search_tree_node_t;
typedef int (*ds_bst_comp_t)(const ds_binary_search_tree_node_t* a, const ds_binary_search_tree_node_t* b, void* ctx);
typedef struct ds_bst_remove_info_t
{
    ds_binary_search_tree_node_t* node_removed;
    ds_binary_search_tree_node_t* node_unlinked;
    ds_binary_search_tree_node_t* node_replaced;
    ds_binary_search_tree_node_t* node_replaced_parent;
    int node_replaced_side;
} ds_bst_remove_info_t;

typedef struct ds_binary_search_tree_t
{
    ds_binary_tree_t bt;
    void* ctx;
    ds_bst_comp_t cmp;
    int duplicate_policy;
} ds_binary_search_tree_t;


typedef enum
{
    DS_COMP_LT = -1, // Less than
    DS_COMP_GT = 1, // Greater than
    DS_COMP_EQ, // Equal
} ds_comp_result_t;

typedef enum
{
    DS_DUP_POLICY_REJECT,
    DS_DUP_POLICY_REDIRECT_LEFT,
    DS_DUP_POLICY_REDIRECT_RIGHT,
    DS_DUP_POLICY_ACCEPT,
} ds_duplicate_policy_t;