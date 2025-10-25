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
    u32 acpi,
    mmu_region_criterion_t can_insert
);
void mmu_map_fix_insert(memory_region_node_t** head, memory_region_node_t* new_node);

memory_region_node_t* mmu_map_delete_region
(
    memory_region_node_t* head,
    u64 base,
    u64 length,
    mmu_region_criterion_t can_delete
);

void mmu_map_init(memory_info_t* info, void* pool)
{
    mmu_map_util_data.data_pool = (u8*)pool;
    mmu_map_util_data.free_stack_pool = (u32*)((u8*)pool+MAX_POOL_SIZE);

    memset(mmu_map_util_data.data_pool, 0, MAX_POOL_SIZE);

    memset(mmu_map_util_data.free_stack_pool, 0, MAX_REGION_COUNT);
    STACK_INIT_CONT(mmu_map_util_data.free_stack, mmu_map_util_data.free_stack_pool);

    mmu_map_spawn_nil_node();

    // Import memory info

    // TODO
}

void mmu_map_spawn_nil_node()
{
    if (mmu_map_util_data.nil_node) return;
    
    mmu_map_util_data.nil_node = mmu_map_spawn_node(
        0, 0, 0, 0, NODE_COLOR_BLACK
    );

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

    memory_region_node_t* new_node = (memory_region_node_t*)mmu_map_util_data.data_pool[index*sizeof(memory_region_node_t)];

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

    if (node_addr < &mmu_map_util_data.data_pool[0] || 
        node_addr > &mmu_map_util_data.data_pool[MAX_REGION_COUNT*sizeof(memory_region_node_t)]
    ) return;

    u32 index = (node_addr-&mmu_map_util_data.data_pool[0])/sizeof(memory_region_node_t);
    STACK_PUSH(mmu_map_util_data.free_stack, u32, index);

    memset(node_addr, 0, sizeof(memory_region_node_t));
}

memory_region_node_t* mmu_map_insert_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi,
    mmu_region_criterion_t can_insert
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

    while (current->parent != *head && current->parent->other.color == NODE_COLOR_RED)
    {
        // Obtain uncle node
        memory_region_node_t* uncle;
        if (current->parent->parent->left == current->parent)
        uncle = current->parent->parent->right;
        else uncle = current->parent->left;

        // Case 1: Uncle is RED
        if (uncle->other.color == NODE_COLOR_RED)
        {
            // Recolor both parent and uncle to black
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
            
        }
    }

    // Fix root violation ("Root node is always BLACK")
    (*head)->other.color=NODE_COLOR_BLACK;
}