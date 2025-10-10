#include <libk/mmu/mmu_map.h>
#include <libk/utils/algs.h>
#include <libk/containers/stack.h>
#include <libk/string.h>

static struct
{
    memory_region_node_t* main_head;
    u32 current_node_count;
    STACK(u32) free_stack;
    u32 free_stack_pool[MAX_REGION_COUNT];
    u8 pool[MAX_REGION_COUNT*sizeof(memory_region_node_t)];
} mmu_map_data;

static const char* const mem_types[] = {
    "Free",
    "Reserved",
    "ACPI-reclaimable",
    "ACPI non-volatile",
    "Bad",
    "System reserved",
    "Hole",
    "Unknown",
};

void mmu_map_init(memory_info_t* info)
{
    memset(mmu_map_data.pool, 0, sizeof(memory_region_node_t)*MAX_REGION_COUNT);
    memset(mmu_map_data.free_stack_pool, 0, MAX_REGION_COUNT);

    STACK_INIT_CONT(mmu_map_data.free_stack, mmu_map_data.free_stack_pool);

    // Import memory info

    memory_region_node_t* current = mmu_map_data.main_head;
    u32 size = info->entries_count<MAX_REGION_COUNT?info->entries_count:MAX_REGION_COUNT;
    for (u32 i=0;i<size;i++)
    {
        memory_region_t* raw_region = &info->regions[i];
        memory_region_node_t* new_node = mmu_map_create_node(
            raw_region->base,
            raw_region->length,
            raw_region->type,
            raw_region->acpi
        );

        if (!new_node) break;

        if (!mmu_map_data.main_head)
        {
            mmu_map_data.main_head=new_node;
        }
        else
        {
            current->next = new_node;
            new_node->prev=current;
        }
        current = new_node;
    }
}

void mmu_map_traverse_region()
{
    memory_region_node_t* current = mmu_map_data.main_head;
    while (current)
    {
        current=current->next;
    }
}

memory_region_node_t* mmu_map_split_region(    
    memory_region_node_t* node, 
    u64 base 
)
{
    if (!node) return NULL;

    if (base <= node->region.base) return node;

    u64 base_end = node->region.base+node->region.length;

    memory_region_node_t* node_new = mmu_map_create_node
    (
        base, 
        base_end-base, 
        MEMORY_TYPE_UNIDENTIFIED, 
        0
    );

    if (!node_new) return NULL;

    node_new->next = node->next;
    node_new->prev = node;

    node->next=node_new;

    return node_new;
}

void mmu_map_merge_region_forward(    
    memory_region_node_t* node
)
{
    if (!node || !node->next) return NULL;

    memory_region_node_t* to_erase = node->next;

    node->next=node->next->next;

    if (node->next)
    node->next->prev=node;

    node->region.length = (to_erase->region.base + to_erase->region.length) - node->region.base;

    mmu_map_erase_node(to_erase);
}

void mmu_map_merge_region_backward(    
    memory_region_node_t* node
)
{
    if (!node || !node->prev) return NULL;

    memory_region_node_t* to_erase = node->prev;

    node->prev=node->prev->prev;

    if (node->prev)
    node->prev->next=node;
    
    node->region.length = (to_erase->region.base + to_erase->region.length) - node->region.base;

    mmu_map_erase_node(to_erase);
}


memory_region_node_t* mmu_map_create_node(
    u64 base, 
    u64 length, 
    u32 type,
    u32 acpi
)
{
    u32 index;
    if (!STACK_EMPTY(mmu_map_data.free_stack))
    {
        index = STACK_TOP(mmu_map_data.free_stack, u32);
        STACK_POP(mmu_map_data.free_stack);
    }
    else 
    {
        if (mmu_map_data.current_node_count >= MAX_REGION_COUNT)
            return NULL;

        index = mmu_map_data.current_node_count++;
    }

    memory_region_node_t* new_node = mmu_map_data.pool[index*sizeof(memory_region_node_t)];

    new_node->next=NULL; new_node->prev=NULL;
    new_node->region = (memory_region_t){
        .base = base,
        .length = length,
        .acpi = acpi,
        .type = type
    };

    return new_node;
}

void mmu_map_erase_node(memory_region_node_t* node_addr)
{
    if (!node_addr) return;

    if (node_addr < &mmu_map_data.pool[0] || 
        node_addr > &mmu_map_data.pool[MAX_REGION_COUNT*sizeof(memory_region_node_t)]
    ) return;

    u32 index = (node_addr-&mmu_map_data.pool[0])/sizeof(memory_region_node_t);
    STACK_PUSH(mmu_map_data.free_stack, u32, index);

    memset(node_addr, 0, sizeof(memory_region_t));
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
    if (!can_insert) return NULL;

    if (!head)
    {
        head = mmu_map_create_node(base, length, type, acpi);
        return head;
    }

    memory_region_node_t* current = head;
    u64 current_base, current_base_end;

    while (current)
    {
        if (!can_insert(current)) goto next_ins;

        current_base = current->region.base;
        current_base_end = current->region.base+current->region.length;
        if (base >= current_base && base < current_base_end)
            break;
next_ins:        current=current->next;
    }
    if (!current) return NULL;

    memory_region_node_t* left=current;

    if (base > current_base)
    {
        left = mmu_map_split_region(current, base);
        if (!left) return NULL;
    }
    
    memory_region_node_t* right=left;
    if (base+length < current_base_end)
    {
        right = mmu_map_split_region(left, base+length);
        if (!right) return NULL;
    }
    
    left->region.type = type;
    left->region.acpi = acpi;
    
    if (left->next && left->next->region.type == left->region.type)
        mmu_map_merge_region_forward(left);
    if (left->prev && left->prev->region.type == left->region.type)
        mmu_map_merge_region_backward(left);

    return left;
}

memory_region_node_t* mmu_map_delete_region
(
    memory_region_node_t* head, 
    u64 base, 
    u64 length,
    u32 type, 
    u32 acpi,
    mmu_region_criterion_t can_delete
)
{
    if (!can_delete) return NULL;

    if (!head) return NULL;

    memory_region_node_t* current = head;
    u64 current_base, current_base_end; 
    bool merge_split_switch=false; // True for Merge, False for Split

    while (current)
    {
        if (!can_delete(current)) goto next_del;
        if (current->region.type != type || current->region.acpi != acpi) goto next_del;

        current_base = current->region.base;
        current_base_end = current->region.base+current->region.length;
        
        // Case 1: Range [base, base+length) is exactly the same as [current_base, current_base_end)
        if (base==current_base && base+length == current_base_end)
        {
            merge_split_switch = true;
            break;
        }

        // Case 2: Range [base, base+length) is entirely inside [current_base, current_base_end)
        if (base >= current_base && base+length < current_base_end)
            break;
next_del:        current=current->next;
    }
    if (!current) return NULL;

    if (merge_split_switch)
    {
        memory_region_node_t* next_in_merge = current;
        if (current->prev && current->prev->region.type == MEMORY_TYPE_FREE)
        {
            next_in_merge = current->prev;
            mmu_map_merge_region_forward(next_in_merge); 
        }

        if (current->next && current->next->region.type == MEMORY_TYPE_FREE)
        {
            next_in_merge = current->next;
            mmu_map_merge_region_backward(next_in_merge); 
        }
    }
    else
    {
        memory_region_node_t* left=current;

        if (base > current_base)
        {
            left = mmu_map_split_region(current, base);
            if (!left) return NULL;
        }
        
        memory_region_node_t* right=left;
        if (base+length < current_base_end)
        {
            right = mmu_map_split_region(left, base+length);
            if (!right) return NULL;
        }
        
        left->region.type = MEMORY_TYPE_FREE;
        left->region.acpi = 0;
        
        if (left->next && left->next->region.type == MEMORY_TYPE_FREE)
        {
            mmu_map_merge_region_forward(left);
        }
        if (left->prev && left->prev->region.type == MEMORY_TYPE_FREE)
        {
            mmu_map_merge_region_backward(left);
        }
    }

    return head;
}
