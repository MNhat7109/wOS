#include <tree.h>


void ds_tree_init(ds_tree_t* tree)
{
    tree->root = NULL;
}

void ds_tree_node_init(ds_tree_node_t* node)
{
    node->parent = node->next_sibling = node->first_child = NULL;
}

int ds_tree_attach(ds_tree_t* tree, ds_tree_node_t* parent, ds_tree_node_t* node)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    if (!parent)
    {
        tree->root = node;
        return DS_STATUS_SUCCESS;
    }

    ds_tree_node_t* prev = parent->first_child;
    while (prev->next_sibling) prev = prev->next_sibling;

    prev->next_sibling = node;
    node->parent = parent;
    return DS_STATUS_SUCCESS;
}

int ds_tree_detach(ds_tree_t* tree, ds_tree_node_t* node)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    if (!node->parent)
    {
        tree->root = NULL;
        return DS_STATUS_SUCCESS;
    }

    ds_tree_node_t* prev;
    for (prev = node->parent->first_child; prev; prev=prev->next_sibling)
    {
        if (prev->next_sibling==node) break;
    }
    if (!prev) return -DS_TREE_DETACH_NOT_FOUND;

    prev->next_sibling = node->next_sibling;
    node->parent = NULL;
    
    return DS_STATUS_SUCCESS;
}