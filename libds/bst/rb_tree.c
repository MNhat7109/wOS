#include <rb_tree.h>
#include <bst.h>
#include <binary_tree.h>
#include <stdbool.h>

// RED-BLACK TREE RULES
// 1. Nodes are either BLACK or RED.
// 2. Root is BLACK.
// 3. NIL nodes are BLACK.
// 4. If a node is RED, both of its children must be BLACK.
// 5. Every path from node to NIL must have the same bh values*

// *bh(x): Black-height of x, the number of black nodes on the path from x->NIL node, without counting x itself

typedef enum
{
    DS_COLOR_RED,
    DS_COLOR_BLACK,
} ds_rb_color_t; // Rule 1

typedef enum
{
    DS_SIDE_LEFT = -1,
    DS_SIDE_ROOT = 0,
    DS_SIDE_RIGHT = +1,
    DS_SIDE_UNK = 2,
} ds_side_t;

static int ds_rb_tree_side_of(ds_rb_tree_node_t* node)
{
    return ds_binary_tree_side_of(&node->bst);
}

static int ds_rb_tree_color_of(ds_rb_tree_node_t* node)
{
    return node?node->color:DS_COLOR_BLACK; // Rule 3
}

ds_rb_tree_node_t* ds_rb_tree_convert_raw(ds_binary_search_tree_node_t* node)
{
    return container_of(node, ds_rb_tree_node_t, bst);
}

int ds_rb_tree_fix_violations_upon_insertion(ds_rb_tree_t* tree, ds_rb_tree_node_t* node);
int ds_rb_tree_fix_violations_upon_deletion(ds_rb_tree_t* tree, 
    ds_rb_tree_node_t* replacement_parent, ds_rb_tree_node_t* replacement, int side);

void ds_rb_tree_init(ds_rb_tree_t* tree, void* ctx, ds_bst_comp_t cmp, int dup_policy)
{
    ds_binary_search_tree_init(tree, ctx, cmp, dup_policy);
}

void ds_rb_tree_node_init(ds_rb_tree_node_t* node)
{
    if (!node) return;
    node->bst = (ds_binary_search_tree_node_t){
        .left = NULL,
        .right = NULL,
        .parent = NULL
    };
    node->color = DS_COLOR_RED; // Rule 4
}

int ds_rb_tree_attach(ds_rb_tree_t* tree, ds_rb_tree_node_t* node)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;
    
    int status;

    // Add node like BST
    status = ds_binary_search_tree_attach(tree, &node->bst);
    if (status < 0) return status;

    // Fix violations
    status = ds_rb_tree_fix_violations_upon_insertion(tree, node);
    if (status < 0) return status;

    return DS_STATUS_SUCCESS;
}

int ds_rb_tree_remove(ds_rb_tree_t* tree, ds_rb_tree_node_t* node, ds_bst_remove_info_t* addl_info)
{
    if (!tree || !node) return -DS_STATUS_INVALID_INPUT;

    int status;
    
    // Remove node like BST
    ds_bst_remove_info_t remove_info;
    status = ds_binary_search_tree_remove(tree, &node->bst, &remove_info);
    int deleted_color = ds_rb_tree_color_of(ds_rb_tree_convert_raw(remove_info.node_removed));
    if (status < 0) return status;

    ds_rb_tree_node_t* rb_unlinked = ds_rb_tree_convert_raw(remove_info.node_unlinked);
    int unlinked_color = ds_rb_tree_color_of(rb_unlinked);

    if (remove_info.node_unlinked != remove_info.node_removed) 
    {
        rb_unlinked->color = deleted_color;
    }

    // Fix violations
    if (unlinked_color == DS_COLOR_RED) return DS_STATUS_SUCCESS;

    status = ds_rb_tree_fix_violations_upon_deletion(tree, 
        ds_rb_tree_convert_raw(remove_info.node_replaced_parent), 
        ds_rb_tree_convert_raw(remove_info.node_replaced), 
        remove_info.node_replaced_side
    );
    if (status < 0) return status;

    if (addl_info) *addl_info = remove_info;

    return DS_STATUS_SUCCESS;
}

ds_rb_tree_node_t* ds_rb_tree_find(ds_rb_tree_t* tree, ds_rb_tree_node_t* node)
{
    // Same as bst_find(), but faster as tree has already been balanced after insertion/deletion
    return ds_rb_tree_convert_raw(ds_binary_search_tree_search(tree, &node->bst));
}

int ds_rb_tree_fix_violations_upon_insertion(ds_rb_tree_t* tree, ds_rb_tree_node_t* node)
{
    // There are 2 ways to correct RB TREE RULE violations: recoloring, and rotation
    // Either one of these two is enough to correct the tree, but sometimes both are needed to do so
    if (!tree || !node) return -1;
    if (!tree->bt.root) return -1;

    ds_rb_tree_node_t* rb_parent_node = ds_rb_tree_convert_raw(node->bst.parent);
    ds_rb_tree_node_t* rb_root = ds_rb_tree_convert_raw(tree->bt.root);
    
    while (ds_rb_tree_color_of(rb_parent_node) == DS_COLOR_RED)
    {
        ds_rb_tree_node_t* rb_gp_node = ds_rb_tree_convert_raw(rb_parent_node->bst.parent);
        int p_side = ds_rb_tree_side_of(rb_parent_node);
        if (p_side == DS_SIDE_LEFT)
        {
            ds_rb_tree_node_t* rb_unc_node = ds_rb_tree_convert_raw(rb_gp_node->bst.right);
            
            if (ds_rb_tree_color_of(rb_unc_node) == DS_COLOR_RED)
            {
                // Recoloring of grandparent and both of its children is needed
                rb_unc_node->color = DS_COLOR_BLACK;
                rb_parent_node->color = DS_COLOR_BLACK;
                rb_gp_node->color = DS_COLOR_RED;

                node = rb_gp_node;
                rb_parent_node = ds_rb_tree_convert_raw(node->bst.parent);
            }
            else
            {
                int cur_side = ds_rb_tree_side_of(node);
                if (cur_side == DS_SIDE_RIGHT)
                {
                    // L-R
                    ds_rb_tree_node_t* old_parent = rb_parent_node;
                    ds_binary_tree_rotate_left(&tree->bt, &rb_parent_node->bst);

                    // Before rotation: node=N, parent=P
                    // After rotation: node=P, parent=N
                    rb_parent_node = node; 
                    node = old_parent;
                }
                // Both L-L and L-R
                rb_parent_node->color = DS_COLOR_BLACK;
                rb_gp_node->color = DS_COLOR_RED;

                ds_binary_tree_rotate_right(&tree->bt, &rb_gp_node->bst);
            }
        }
        else if (p_side==DS_SIDE_RIGHT)
        {
            ds_rb_tree_node_t* rb_unc_node = ds_rb_tree_convert_raw(rb_gp_node->bst.left);

            if (ds_rb_tree_color_of(rb_unc_node) == DS_COLOR_RED)
            {
                // Recoloring of grandparent and both of its children is needed
                rb_unc_node->color = DS_COLOR_BLACK;
                rb_parent_node->color = DS_COLOR_BLACK;
                rb_gp_node->color = DS_COLOR_RED;

                node = rb_gp_node;
                rb_parent_node = ds_rb_tree_convert_raw(node->bst.parent);
            }
            else
            {
                int cur_side = ds_rb_tree_side_of(node);
                if (cur_side == DS_SIDE_LEFT)
                {
                    // R-L
                    ds_rb_tree_node_t* old_parent = rb_parent_node;
                    ds_binary_tree_rotate_right(&tree->bt, &rb_parent_node->bst);

                    // Before rotation: node=N, parent=P
                    // After rotation: node=P, parent=N
                    rb_parent_node = node; 
                    node = old_parent;
                }
                // Both R-R and R-L
                rb_parent_node->color = DS_COLOR_BLACK;
                rb_gp_node->color = DS_COLOR_RED;

               ds_binary_tree_rotate_left(&tree->bt, &rb_gp_node->bst);
            }
        }
        // Rule 2
        rb_root = ds_rb_tree_convert_raw(tree->bt.root);
        if (node == rb_root) break;
    }
    rb_root->color = DS_COLOR_BLACK;
    return 0;
}

int ds_rb_tree_fix_violations_upon_deletion(ds_rb_tree_t* tree, 
    ds_rb_tree_node_t* replacement_parent, ds_rb_tree_node_t* replacement, int side
)
{
    if (!tree) return -1;

    // There is NO RED-RED, as RED parent are only allowed to have BLACK children (rule 4)

    // BLACK-BLACK: Double-black encountered, now check the node's family tree
    // Rule 5: Black height must be maintained
    ds_rb_tree_node_t* cur = replacement;

    while (cur != ds_rb_tree_convert_raw(tree->bt.root) && ds_rb_tree_color_of(cur) == DS_COLOR_BLACK)
    {
        ds_rb_tree_node_t* parent; int cur_side;
        if (!cur)
        {
            // In case the deleted node is a leaf node
            parent = replacement_parent;
            cur_side = side;
        }
        else
        {
            parent = ds_rb_tree_convert_raw(cur->bst.parent);
            cur_side = ds_rb_tree_side_of(cur);
        }
        
        ds_rb_tree_node_t*sibling, *near_nib, *far_nib;
        // Case 1: Sibling is RED
        // Rotate according to cur's side around parent and swap colors of parent and sibling

        // Case 2: Both niblings (sibling's children) are BLACK
        // => Sibling MUST BE RED. Recolor sibling to RED regardless and propagate up

        // near nibling: Sibling's child that is in the same side as cur
        // far nibling: Sibling's child that is in the different side as cur
        
        // Case 3: Near nilbing is RED
        // - Swap colors between sibling, and the near nibling
        // - Rotate against cur's side around sibling

        // Case 4:
        // - Set sibling's color as parent's color
        // - Set far nibling's color to be BLACK
        // - Rotate according to cur's side around parent
        // - Set cur as root

        if (cur_side == DS_SIDE_LEFT)
        {
            sibling = ds_rb_tree_convert_raw(parent->bst.right);

            if (ds_rb_tree_color_of(sibling) == DS_COLOR_RED)
            {
                sibling->color = DS_COLOR_BLACK;
                parent->color = DS_COLOR_RED;
                ds_binary_tree_rotate_left(&tree->bt, &parent->bst);
                sibling = ds_rb_tree_convert_raw(parent->bst.right);
            }

            near_nib = ds_rb_tree_convert_raw(sibling->bst.left);
            far_nib  = ds_rb_tree_convert_raw(sibling->bst.right);

            if (
                ds_rb_tree_color_of(near_nib) == DS_COLOR_BLACK &&
                ds_rb_tree_color_of(far_nib) == DS_COLOR_BLACK
            )
            {
                sibling->color = DS_COLOR_RED;
                cur = parent;
            }
            else
            {
                if (ds_rb_tree_color_of(far_nib) == DS_COLOR_BLACK)
                {
                    near_nib->color = DS_COLOR_BLACK;
                    sibling->color = DS_COLOR_RED;
                    ds_binary_tree_rotate_right(&tree->bt, &sibling->bst);
                    sibling = ds_rb_tree_convert_raw(parent->bst.right);
                    far_nib = ds_rb_tree_convert_raw(sibling->bst.right);
                }

                sibling->color = parent->color;
                parent->color = DS_COLOR_BLACK;
                far_nib->color = DS_COLOR_BLACK;
                ds_binary_tree_rotate_left(&tree->bt, &parent->bst);

                cur = ds_rb_tree_convert_raw(tree->bt.root);
            }
        }
        else
        {
            sibling = ds_rb_tree_convert_raw(parent->bst.left);

            if (ds_rb_tree_color_of(sibling) == DS_COLOR_RED)
            {
                sibling->color = DS_COLOR_BLACK;
                parent->color = DS_COLOR_RED;
                ds_binary_tree_rotate_right(&tree->bt, &parent->bst);
                sibling = ds_rb_tree_convert_raw(parent->bst.left);
            }

            near_nib = ds_rb_tree_convert_raw(sibling->bst.right);
            far_nib = ds_rb_tree_convert_raw(sibling->bst.left);

            if (
                ds_rb_tree_color_of(near_nib) == DS_COLOR_BLACK &&
                ds_rb_tree_color_of(far_nib) == DS_COLOR_BLACK
            )
            {
                sibling->color = DS_COLOR_RED;
                cur = parent;
            }
            else
            {
                if (ds_rb_tree_color_of(far_nib) == DS_COLOR_BLACK)
                {
                    near_nib->color = DS_COLOR_BLACK;
                    sibling->color = DS_COLOR_RED;
                    ds_binary_tree_rotate_left(&tree->bt, &sibling->bst); // Rotate round sibling
                    sibling = ds_rb_tree_convert_raw(parent->bst.left);
                    far_nib = ds_rb_tree_convert_raw(sibling->bst.left);
                }
                
                sibling->color = parent->color;
                parent->color = DS_COLOR_BLACK;
                far_nib->color = DS_COLOR_BLACK;
                ds_binary_tree_rotate_right(&tree->bt, &parent->bst);
                cur = ds_rb_tree_convert_raw(tree->bt.root);
            }
        }
    }

    // Recolor cur to BLACK, because it's root
    if (cur)
    cur->color = DS_COLOR_BLACK; // Rule 2
    return 0;
}