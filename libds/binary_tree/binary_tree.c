#include <binary_tree.h>

void ds_binary_tree_init(ds_binary_tree_t* binary_tree)
{
    if (!binary_tree) return;
    binary_tree->root = NULL;
    binary_tree->size = 0;
}

void ds_binary_tree_node_init(ds_binary_tree_node_t* node)
{
    if (!node) return;
    node->left = node->right = node->parent = NULL;
}

int ds_binary_tree_side_of(ds_binary_tree_node_t* node)
{
    if (!node) return DS_BTNODE_SIDE_UNK;
    if (!node->parent) return DS_BTNODE_SIDE_ROOT;

    return node->parent->left == node?DS_BTNODE_SIDE_LEFT:DS_BTNODE_SIDE_RIGHT;
}

ds_binary_tree_node_t* ds_binary_tree_rotate_left(ds_binary_tree_t* binary_tree, ds_binary_tree_node_t* pivot)
{
    if (!binary_tree || !pivot) return NULL;

    ds_binary_tree_node_t* right = pivot->right;
    
    pivot->right = right->left;
    if (right->left) right->left->parent = pivot;

    right->parent = pivot->parent;

    if (!pivot->parent) binary_tree->root = right;
    else if (pivot->parent->left == pivot) 
    pivot->parent->left = right;
    else pivot->parent->right = right;

    right->left = pivot;
    pivot->parent = right;
    return right;
}

ds_binary_tree_node_t* ds_binary_tree_rotate_right(ds_binary_tree_t* binary_tree, ds_binary_tree_node_t* pivot)
{
    if (!binary_tree || !pivot) return NULL;

    ds_binary_tree_node_t* left = pivot->left;
    
    pivot->left = left->right;
    if (left->right) left->right->parent = pivot;

    left->parent = pivot->parent;

    if (!pivot->parent) binary_tree->root = left;
    else if (pivot->parent->left == pivot) 
    pivot->parent->left = left;
    else pivot->parent->right = left;

    left->right = pivot;
    pivot->parent = left;
    return left;
}

int ds_binary_tree_transplant(ds_binary_tree_t* binary_tree, 
    ds_binary_tree_node_t* node,
    ds_binary_tree_node_t* replacement
)
{
    if (!binary_tree || !node) return -DS_STATUS_INVALID_INPUT;

    if (!node->parent)
    {
        binary_tree->root = replacement;
    }
    else if (node->parent->left == node) node->parent->left = replacement;
    else node->parent->right= replacement;

    if (replacement)
    {
        replacement->parent = node->parent;
    }
    return DS_STATUS_SUCCESS;
}
