#include <avl_tree.h>
#include <bst.h>
#include <binary_tree.h>

#define max(_a,_b) (((_a)>(_b))?(_a):(_b))
#define abs(_x) (((_x)<0)?-(_x):(_x))

int ds_avl_tree_rebalance(ds_avl_tree_t* tree, ds_avl_tree_node_t* parent);

ds_avl_tree_node_t* ds_avl_tree_convert_raw(ds_binary_search_tree_node_t* node)
{
    return container_of(node, ds_avl_tree_node_t, bst);
}

static ssize_t ds_avl_tree_height_of(ds_avl_tree_node_t* node)
{
    return !node?-1:node->height;
}

static void ds_avl_tree_recalculate_height(ds_avl_tree_node_t* node)
{
    if (!node) return;
    node->height = 1 + max(
            ds_avl_tree_height_of(ds_avl_tree_convert_raw(node->bst.left)),
            ds_avl_tree_height_of(ds_avl_tree_convert_raw(node->bst.right))
        );
}

ssize_t ds_avl_tree_balance_of(ds_avl_tree_node_t* node)
{
    // Returns negative if height(right) < height(left)
    return !node
    ? 0 : 
    ds_avl_tree_height_of(ds_avl_tree_convert_raw(node->bst.right))
    - ds_avl_tree_height_of(ds_avl_tree_convert_raw(node->bst.left));
}

static ds_avl_tree_node_t* ds_avl_tree_rotate_left(ds_avl_tree_t* tree, ds_avl_tree_node_t* pivot)
{
    if (!tree || !pivot) return NULL;
    // Rotate tree in BST level
    ds_binary_search_tree_node_t* new_piv_bst = ds_binary_tree_rotate_left(&tree->bt, &pivot->bst);
    
    ds_avl_tree_recalculate_height(ds_avl_tree_convert_raw(new_piv_bst));
    ds_avl_tree_recalculate_height(ds_avl_tree_convert_raw(new_piv_bst->left));
    return pivot;
}

static ds_avl_tree_node_t* ds_avl_tree_rotate_right(ds_avl_tree_t* tree, ds_avl_tree_node_t* pivot)
{
    if (!tree || !pivot) return NULL;
    // Rotate tree in BST level
    ds_binary_search_tree_node_t* new_piv_bst = ds_binary_tree_rotate_right(&tree->bt, &pivot->bst);
    
    ds_avl_tree_recalculate_height(ds_avl_tree_convert_raw(new_piv_bst));
    ds_avl_tree_recalculate_height(ds_avl_tree_convert_raw(new_piv_bst->right));
    return pivot;
}

void ds_avl_tree_init(ds_avl_tree_t* tree, void* ctx, ds_bst_comp_t cmp, int dup_policy)
{
    ds_binary_search_tree_init(tree, ctx, cmp, dup_policy);
}

void ds_avl_tree_node_init(ds_avl_tree_node_t* node)
{
    if (!node) return;

    node->bst = (ds_binary_search_tree_node_t){
        .left = NULL,
        .right = NULL,
        .parent = NULL
    };
    node->height = 0;
}

int ds_avl_tree_attach(ds_avl_tree_t* tree, ds_avl_tree_node_t* node)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    int status;

    // Perform regular BST attach
    status = ds_binary_search_tree_attach(tree, &node->bst);
    if (status < 0) return status;

    // Init node's height
    node->height = 0;

    // Fix violations
    status = ds_avl_tree_rebalance(tree, ds_avl_tree_convert_raw(node->bst.parent));
    if (status < 0) return status;

    return DS_STATUS_SUCCESS;
}

int ds_avl_tree_remove(ds_avl_tree_t* tree, ds_avl_tree_node_t* node, ds_bst_remove_info_t* addl_info)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    int status; 

    // Perform regular BST remove
    ds_bst_remove_info_t remove_info;
    status = ds_binary_search_tree_remove(tree, &node->bst, &remove_info);
    if (status < 0) return status;
    
    // Fix violations
    status = ds_avl_tree_rebalance(tree, ds_avl_tree_convert_raw(remove_info.node_replaced_parent));
    if (status < 0) return status;

    if (addl_info) *addl_info = remove_info;
    
    return DS_STATUS_SUCCESS;
}

ds_avl_tree_node_t* ds_avl_tree_find(ds_avl_tree_t* tree, ds_avl_tree_node_t* node)
{
    return ds_avl_tree_convert_raw(ds_binary_search_tree_search(tree, &node->bst));
}

int ds_avl_tree_rebalance(ds_avl_tree_t* tree, ds_avl_tree_node_t* node)
{
    if (!tree) return -1;

    ds_avl_tree_node_t* cur = node;
    while (cur)
    {
        ssize_t old_height = cur->height;
        ds_avl_tree_recalculate_height(cur);

        ssize_t balance = ds_avl_tree_balance_of(cur);

        // Rotate the tree according to 4 cases: L-L, L-R, R-L, R-R
        if (balance < -1)
        {
            // Tilted to the left
            ds_avl_tree_node_t* left = ds_avl_tree_convert_raw(cur->bst.left);
            ssize_t left_balance = ds_avl_tree_balance_of(left);
            // L-R
            if (left_balance > 0)
            {
                // Rotation
                ds_avl_tree_rotate_left(tree, left);
            }
            // Rotation for both L-L and L-R
            ds_avl_tree_rotate_right(tree, cur);
        }

        if (balance > 1)
        {
            // Tilted to the right
            ds_avl_tree_node_t* right = ds_avl_tree_convert_raw(cur->bst.right);
            ssize_t right_balance = ds_avl_tree_balance_of(right);
            // R-L
            if (right_balance < 0)
            {
                // Rotation
                ds_avl_tree_rotate_right(tree, right);
            }
            // Rotation for both R-L and R-R
            ds_avl_tree_rotate_left(tree, cur);
        }

        // if (old_height == cur->height) break;
        
        cur = ds_avl_tree_convert_raw(cur->bst.parent);
    }
    return 0;
}
