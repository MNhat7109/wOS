#include <bst.h>
#include <binary_tree.h>

void ds_binary_search_tree_init(
    ds_binary_search_tree_t* tree, 
    void* ctx, 
    ds_bst_comp_t cmp,
    int duplicate_handling
)
{
    ds_binary_tree_init(&tree->bt);
    if (!cmp) return;
    tree->ctx = ctx;
    tree->cmp = cmp;
    tree->duplicate_policy = duplicate_handling;
}

void ds_binary_search_tree_node_init(ds_binary_search_tree_node_t* node)
{
    ds_binary_tree_node_init(node);
}

ds_binary_search_tree_node_t* 
ds_binary_search_tree_successor_of(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node
)
{
    if (!tree || !node) return NULL;

    ds_binary_search_tree_node_t* cur = tree->bt.root, *succ = NULL;
    while (cur)
    {
        if (tree->cmp(cur, node, tree->ctx) == DS_COMP_GT) 
        {
            succ = cur;
            cur = cur->left;
        }
        else cur = cur->right;
    }
    return succ;
}

ds_binary_search_tree_node_t* 
ds_binary_search_tree_predecessor_of(ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node
)
{
    if (!tree || !node) return NULL;

    ds_binary_search_tree_node_t* cur = tree->bt.root, *pred = NULL;
    while (cur)
    {
        if (tree->cmp(cur, node, tree->ctx) == DS_COMP_LT)
        {
            pred = cur;
            cur = cur->right;
        } 
        else cur = cur->left;
    }
    return pred;
}

ds_binary_search_tree_node_t* ds_binary_search_tree_search(
    ds_binary_search_tree_t* tree, 
    ds_binary_search_tree_node_t* node
)
{
    if (!tree || !node) return NULL;

    ds_binary_search_tree_node_t* cur = tree->bt.root;
    while (cur)
    {
        if (tree->cmp(cur, node, tree->ctx) == DS_COMP_EQ) return cur;
        else if (tree->cmp(cur, node, tree->ctx) == DS_COMP_GT) cur = cur->left;
        else cur = cur->right;
    }
    return NULL;
}


int ds_binary_search_tree_attach(ds_binary_search_tree_t* tree, ds_binary_search_tree_node_t* node)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    if (!tree->bt.root)
    {
        tree->bt.root = node;
        goto done;
    }

    // Scour through the tree until we find a leaf
    ds_binary_search_tree_node_t* to_add = tree->bt.root;
    while (to_add)
    {
        ds_binary_search_tree_node_t* next; 
        int status = tree->cmp(node, to_add, tree->ctx);
        if (status == DS_COMP_EQ) 
        {
            switch (tree->duplicate_policy)
            {
                case DS_DUP_POLICY_REJECT: return -DS_BST_INSERT_DUP_NOT_ALLOWED;
                case DS_DUP_POLICY_REDIRECT_LEFT:
                    status = DS_COMP_LT;
                    break;
                default:
                    status = DS_COMP_GT;
                    break;
            }
        }
        
        if (status == DS_COMP_LT) next = to_add->left;
        else next = to_add->right;

        if (!next) break;
        to_add = next;
    }
    
    // Attach the node to that leaf
    if (tree->cmp(node, to_add, tree->ctx) == DS_COMP_LT) to_add->left = node;
    else to_add->right = node;

    node->parent = to_add;
done:
    // Yeah, that's it.
    tree->bt.size++;
    return DS_STATUS_SUCCESS;
}

int ds_binary_search_tree_remove(ds_binary_search_tree_t* tree, ds_binary_search_tree_node_t* target
, ds_bst_remove_info_t* addl_info)
{
    if (!tree || !target) return -DS_STATUS_INVALID_INPUT;

    if (!tree->bt.root) return -DS_BST_DELETE_ON_NULL_ROOT;

    ds_binary_search_tree_node_t* node = ds_binary_search_tree_search(tree, target);
    if (!node) return -DS_BST_DELETE_NOT_FOUND;

    ds_binary_search_tree_node_t* removed=node,
    *to_be_unlinked, *replacement, *replacement_parent;
    int replacement_side;
    
    // Case 1: Node has zero or one child
    // Replace node with its only child
    if (!node->left)
    {
        to_be_unlinked = node;
        replacement = node->right;
        replacement_parent = node->parent;
        replacement_side = ds_binary_tree_side_of(node);
        ds_binary_tree_transplant(&tree->bt, node, node->right);
    }
    else if (!node->right)
    {
        to_be_unlinked = node;
        replacement = node->left;
        replacement_parent = node->parent;
        replacement_side = ds_binary_tree_side_of(node);
        ds_binary_tree_transplant(&tree->bt, node, node->left);
    }
    else
    {
        to_be_unlinked = ds_binary_search_tree_successor_of(tree, node);
        replacement = to_be_unlinked->right;
        replacement_parent= to_be_unlinked->parent;
        replacement_side = ds_binary_tree_side_of(to_be_unlinked);

        if (to_be_unlinked->parent == node)
        {
            if (replacement) 
            {
                replacement->parent = to_be_unlinked;
            }
            replacement_parent = to_be_unlinked;
        }
        else
        {
            ds_binary_tree_transplant(&tree->bt, to_be_unlinked, to_be_unlinked->right);
            to_be_unlinked->right = node->right;
            to_be_unlinked->right->parent = to_be_unlinked;
        }
        ds_binary_tree_transplant(&tree->bt, node, to_be_unlinked);
        to_be_unlinked->left = node->left;
        to_be_unlinked->left->parent = to_be_unlinked;
    }

    if (!addl_info) goto invalidate;

    addl_info->node_removed = removed;
    addl_info->node_unlinked = to_be_unlinked;
    addl_info->node_replaced = replacement;
    addl_info->node_replaced_parent = replacement_parent;
    addl_info->node_replaced_side = replacement_side;

invalidate:
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;

    tree->bt.size--;
    return DS_STATUS_SUCCESS;    
}
