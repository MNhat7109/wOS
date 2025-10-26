#include <libk/stdint.h>
#include <stdbool.h>
#include <libk/containers/stack.h>
#include <libk/mmu/mmu_map.h>

typedef struct memory_region_node_t memory_region_node_t;

#define MAX_POOL_SIZE MAX_REGION_COUNT*sizeof(memory_region_node_t)
#define NODE_MAGIC_NIL 0xFF
#define NODE_NIL mmu_map_util_data.nil_node

static struct
{
    memory_region_node_t* main_head;
    memory_region_node_t* nil_node;
    u32 current_node_count;
    STACK(u32) free_stack;
    u32* free_stack_pool;
    u8* data_pool;
} mmu_map_util_data;

typedef enum
{
    NODE_COLOR_RED, NODE_COLOR_BLACK
} rb_tree_color_t;


typedef struct memory_region_node_t
{
    u64 base, length;
    struct node_attr_t
    {
        u8 nil_check;
        u8 type : 6;
        u8 acpi : 1;
        u8 color : 1;
    } __attribute__((packed)) other;
    memory_region_node_t *parent;
    memory_region_node_t *left, *right;
} memory_region_node_t;

void mmu_map_spawn_nil_node();
memory_region_node_t* mmu_map_spawn_node(
    u64 base, 
    u64 length, 
    u32 type,
    u32 acpi,
    u32 color
);
void mmu_map_delete_node(memory_region_node_t* node_addr);

memory_region_node_t* mmu_map_insert_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi
);
void mmu_map_fix_insert(memory_region_node_t** head, memory_region_node_t* new_node);

void mmu_map_rotate_node_left(memory_region_node_t** head, memory_region_node_t* pivot);
void mmu_map_rotate_node_right(memory_region_node_t** head, memory_region_node_t* pivot);

memory_region_node_t* mmu_map_delete_region
(
    memory_region_node_t* head,
    u64 base,
    u64 length
);

void mmu_map_init(memory_info_t* info, void* pool)
{
    // Initiate memory pool for map storing
    mmu_map_util_data.data_pool = (u8*)pool; // Data pool for nodes

    // To prevent fragments when freeing and spawning nodes as "bump allocators",
    // We'll need a free list to store all of the freed node addresses
    // And as all nodes are of same size, we can use a stack for it, making the activity of retrieving free block O(1).
    mmu_map_util_data.free_stack_pool = (u32*)((u8*)pool+MAX_POOL_SIZE);

    // Zero out blocks, and initialize stack API for later use
    memset(mmu_map_util_data.data_pool, 0, MAX_POOL_SIZE);
    memset(mmu_map_util_data.free_stack_pool, 0, MAX_REGION_COUNT*sizeof(u32));
    STACK_INIT_CONT(mmu_map_util_data.free_stack, mmu_map_util_data.free_stack_pool);

    // Spawns NIL node for later use. This node will be important, as all phantom leaves
    // point to the address of it.
    mmu_map_spawn_nil_node();

    // Import memory info
    usize item_size = (info->entries_count>=MAX_REGION_COUNT)?MAX_REGION_COUNT:info->entries_count;

    for (usize i=0;i<item_size;i++)
    {
        struct compatible_memregion_t* region = &info->regions[i];

        mmu_map_util_data.main_head = mmu_map_insert_region(
            mmu_map_util_data.main_head, 
            region->base, 
            region->length,
            region->type,
            region->acpi
        );
    }
}

void mmu_map_spawn_nil_node()
{
    if (mmu_map_util_data.nil_node) return;
    
    mmu_map_util_data.nil_node = mmu_map_spawn_node(
        0, 0, 0, 0, NODE_COLOR_BLACK
    );

    // Self reference
    mmu_map_util_data.nil_node->left  = mmu_map_util_data.nil_node;
    mmu_map_util_data.nil_node->right = mmu_map_util_data.nil_node;
    mmu_map_util_data.nil_node->parent = mmu_map_util_data.nil_node;

    // Add NIL check just to be sure
    mmu_map_util_data.nil_node->other.nil_check = NODE_MAGIC_NIL;
}

memory_region_node_t* mmu_map_spawn_node(
    u64 base, 
    u64 length, 
    u32 type,
    u32 acpi,
    u32 color
)
{
    u32 index;
    if (!STACK_EMPTY(mmu_map_util_data.free_stack))
    {
        index = STACK_TOP(mmu_map_util_data.free_stack, u32);
        STACK_POP(mmu_map_util_data.free_stack);
    }
    else 
    {
        if (mmu_map_util_data.current_node_count >= MAX_REGION_COUNT)
            return NULL;

        index = mmu_map_util_data.current_node_count++;
    }

    memory_region_node_t* new_node = (memory_region_node_t*)(mmu_map_util_data.data_pool+index*sizeof(memory_region_node_t));

    new_node->parent=new_node->left=new_node->right=NODE_NIL;
    new_node->base = base; new_node->length = length;
    new_node->other = (struct node_attr_t){
        .nil_check = 0,
        .color = color,
        .acpi = acpi,
        .type = type
    };

    return new_node;
}

void mmu_map_delete_node(memory_region_node_t* node_addr)
{
    if (!node_addr) return;

    if (node_addr <= mmu_map_util_data.data_pool || 
        node_addr > (mmu_map_util_data.data_pool+MAX_REGION_COUNT*sizeof(memory_region_node_t))
    ) return;

    u32 index = (node_addr-mmu_map_util_data.data_pool)/sizeof(memory_region_node_t);
    STACK_PUSH(mmu_map_util_data.free_stack, u32, index);

    memset(node_addr, 0, sizeof(memory_region_node_t));
}

memory_region_node_t* mmu_map_insert_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi
)
{
    // Firstly, insert the node like it's in a BST.
    // Greater than parent to right, less than parent to left

    // We will compare the nodes by its starting base address.

    memory_region_node_t* parent=NULL, *current = head, *new_node=NULL;

    while (current!=NODE_NIL)
    {
        parent=current;

        // We will have 3 cases:

        // Case 1: The node to be added is greater than the current node.
        // In this case, just move the scope to current's right child.
        if (base > current->base) current=current->right;

        // Case 2: The node to be added is less than the current node.
        // In this case, just move the scope to current's left child.
        else if (base < current->base) current=current->left;

        // Case 3: The node is exactly the same 
        // In this case, just return that found node
        else
        {
            new_node=current;
            break;
        }
    }

    // Here, if the needed node already exists, we don't need to spawn a new one
    // Otherwise, create a new node.

    if (!new_node)
    {
        new_node = mmu_map_spawn_node(base, length, type, acpi, NODE_COLOR_RED);
    
        // Link!
        new_node->parent = parent;
        
        if (!parent)
        {
            new_node->other.color = NODE_COLOR_BLACK;
            new_node->parent = NODE_NIL;
            head = new_node;
        }
        else
        {
            if (base < parent->base) parent->left = new_node;
            else parent->right = new_node;
        }
    
        // Then, fix things that we may have violated after the insert
        mmu_map_fix_insert(&head, new_node);
    }

    return head;
}

void mmu_map_fix_insert(memory_region_node_t** head, memory_region_node_t* new_node)
{
    // Fix violations if only the parent node is RED,
    // because the newly inserted node is also RED, they violates the
    // "RED node don't have BLACK children" rule.

    memory_region_node_t* current = new_node;

    while (current != *head && current->parent && current->parent->other.color == NODE_COLOR_RED)
    {
        // Obtain uncle node
        memory_region_node_t* uncle;
        if (current->parent->parent->left == current->parent)
        uncle = current->parent->parent->right;
        else uncle = current->parent->parent->left;

        // Case 1: Uncle is RED
        if (uncle->other.color == NODE_COLOR_RED)
        {
            // Recolor both parent and uncle to BLACK
            current->parent->other.color = NODE_COLOR_BLACK;
            uncle->other.color = NODE_COLOR_BLACK;

            // Recolor grandparent to RED
            current->parent->parent->other.color = NODE_COLOR_RED;
            
            // Move the scope to grandparent
            current = current->parent->parent;
        }

        // Case 2: Uncle is BLACK
        else
        {
            // There will be 2 subcases:

            if (current->parent->left == current)
            {
                // Subcase 2.1: Right-Left
                // This case is true only if the current node is a left child of a right node

                if (current->parent->parent->right == current->parent)
                {
                    // Rotate right on parent
                    mmu_map_rotate_node_right(head, current->parent); 

                    // Move scope up to newly promoted node
                    current=current->parent;
                }

                // Subcase 2.2: Left-Left
                // This case is true only if the current node is a left child of a left node

                if (current->parent->parent->left == current->parent)
                {
                    // Swap colors between grandparent and parent
                    
                    bool old_gp_color = current->parent->parent->other.color;
                    current->parent->parent->other.color = current->parent->other.color;
                    current->parent->other.color = old_gp_color;
                    
                    // Rotate right on grandparent
                    mmu_map_rotate_node_right(head, current->parent->parent);
                }
            }

            // Same applies if the current node is parent's right child
            else
            {
                // Subcase 2.1: Left-Right
                // This case is true only if the current node is a right child of a left node

                if (current->parent->parent->left == current->parent)
                {
                    // Rotate left on parent
                    mmu_map_rotate_node_left(head, current->parent); 

                    // Move scope up to newly promoted node
                    current=current->parent;
                }

                // Subcase 2.2: Right-Right
                // This case is true only if the current node is a right child of a right node

                if (current->parent->parent->right == current->parent)
                {
                    // Swap colors between grandparent and parent
                    
                    bool old_gp_color = current->parent->parent->other.color;
                    current->parent->parent->other.color = current->parent->other.color;
                    current->parent->other.color = old_gp_color;

                    // Rotate left on grandparent
                    mmu_map_rotate_node_left(head, current->parent->parent);
                }    
            }
        }
    }

    // Fix root violation ("Root node is always BLACK")
    (*head)->other.color=NODE_COLOR_BLACK;
}

void mmu_map_rotate_node_left(memory_region_node_t** head, memory_region_node_t* pivot)
{
    memory_region_node_t* right = pivot->right;

    // Move right child's left subtree to the right of pivot
    pivot->right = right->left;

    // Change the existing left subtree ownership from right child to right child's parent (the pivot)
    if (right->left != NODE_NIL)
    {
        right->left->parent = pivot;
    }

    // Link right child to its grandparent
    right->parent = pivot->parent;

    // If pivot is root, switch existing root scope to its right child
    if (pivot->parent == NODE_NIL)
    {
        *head = right;
    }

    // If pivot is left child of its parent, link left of pivot's parent to pivot's right child
    else if (pivot->parent->left == pivot)
    {
        pivot->parent->left = right;
    }

    // Otherwise pivot is parent's right child, link parent's right to right child
    else pivot->parent->right = right;

    // Right child's left child becomes the pivot
    right->left = pivot;

    // And the pivot's parent becomes its right child, making the pivot officially its right child's child
    // NOTE: Damn, this is just as confusing as an average Alabamian family tree

    pivot->parent = right;
}

void mmu_map_rotate_node_right(memory_region_node_t** head, memory_region_node_t* pivot)
{
    memory_region_node_t* left = pivot->left;

    // Move left child's right subtree to the left of pivot
    pivot->left = left->right;

    // Change the existing right subtree ownership from left child to left child's parent (the pivot)
    if (left->right->parent != NODE_NIL)
    {
        left->right->parent = pivot;
    }

    // Link left child to its grandparent
    left->parent = pivot->parent;

    // If pivot is root, move root scope to left child
    if (pivot->parent==NODE_NIL)
    {
        *head = left;
    }

    // If pivot is a left child of its parent, link the pivot's left child to it
    else if (pivot->parent->left == pivot)
    {
        pivot->parent->left = left;
    }

    // Otherwise, link aforementioned left child to be the right child
    else pivot->parent->right = left;

    // Left child's right child becomes the pivot
    left->right = pivot;

    // And the pivot's parent becomes the left child
    pivot->parent = left;
}
