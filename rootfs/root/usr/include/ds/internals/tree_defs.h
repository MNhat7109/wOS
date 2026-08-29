#pragma once
#include "ds_internals.h"

typedef enum
{
    DS_TREE_DETACH_NOT_FOUND = 2
} ds_tree_status_t;

typedef struct ds_tree_node_t ds_tree_node_t;
typedef struct ds_tree_t ds_tree_t;

typedef struct ds_tree_node_t
{
    ds_tree_node_t* parent, *next_sibling, *first_child;
} ds_tree_node_t;

typedef struct ds_tree_t
{
    ds_tree_node_t* root;
} ds_tree_t;