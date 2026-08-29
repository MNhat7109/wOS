#pragma once
#include "ds_internals.h"

typedef struct ds_binary_tree_node_t ds_binary_tree_node_t;
typedef struct ds_binary_tree_t ds_binary_tree_t;

typedef struct ds_binary_tree_node_t
{
    ds_binary_tree_node_t* parent;
    ds_binary_tree_node_t* left;
    ds_binary_tree_node_t* right;
} ds_binary_tree_node_t;

typedef enum
{
    DS_BTNODE_SIDE_LEFT = -1,
    DS_BTNODE_SIDE_ROOT = 0,
    DS_BTNODE_SIDE_RIGHT = +1,
    DS_BTNODE_SIDE_UNK = 2,
} ds_binary_tree_node_side_t;

typedef struct ds_binary_tree_t 
{
    size_t size;
    ds_binary_tree_node_t* root;
} ds_binary_tree_t;
