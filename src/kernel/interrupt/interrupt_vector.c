#include <kernel/interrupt/interrupt_vector.h>

#include <kernel/mmu_heap.h>
#include <kernel/debug.h>
#include <ds/list.h>

static void swap(void* a, void* b) { void* _m = (a);  a = (b); b = (_m); }

#define MAX_DISPATCHER_COUNT 4
typedef struct int_dispatch_blk_list_t
{
    usize size;
    int priority_id;
    ds_list_t list;
} int_dispatch_blk_list_t;

extern struct irq_data_t irq_data;

typedef struct int_dispatch_linked_t
{
    int_dispatch_blk_t dispatch;
    ds_list_node_t topo;
} int_dispatch_linked_t;

static int int_vector_count, int_vector_offset;

static int_dispatch_blk_list_t* dispatch_array;

static int* priority_list;

static void interrupt_vector_set_higher_priority(int_dispatch_blk_list_t* vector_list);
static void interrupt_vector_set_lower_priority (int_dispatch_blk_list_t* vector_list);

int interrupt_vector_alloc_init(int vector_offset, int vector_count)
{
    kdebugf(DEBUG_INFO, MODULE_INT, "Setting up vector allocator...\n");
    int_vector_offset = vector_offset;
    int_vector_count = vector_count;

    // Set up an array of linked dispatches
    dispatch_array = (int_dispatch_blk_list_t*)mmu_heap_alloc(int_vector_count*sizeof(*dispatch_array));
    if (!dispatch_array) return -1;

    // Set up a priority list to keep track of the unused/free vectors
    priority_list = (int*)mmu_heap_alloc(int_vector_count*sizeof(*priority_list));
    if (!priority_list) return -1;
    
    for (int i=0;i<int_vector_count;i++)
    {
        dispatch_array[i].size = 0;
        ds_list_init(&dispatch_array[i].list);
        priority_list[i] = int_vector_offset+i;
        dispatch_array[i].priority_id = i;
    }
    kdebugf(DEBUG_INFO, MODULE_INT, "%d vector(s) available starting from offset 0x%x\n", int_vector_count, int_vector_offset);
}

int interrupt_vector_alloc()
{
    int current_vector_offset = priority_list[0];
    int_dispatch_blk_list_t* list = &dispatch_array[current_vector_offset-int_vector_offset];
    if (list->size >= MAX_DISPATCHER_COUNT) return -1;
    return current_vector_offset;
}

int interrupt_vector_follow_dispatch_chain(int vector, register_state_t* regs)
{
    if (vector < int_vector_offset) return -1;
    int_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    int handled_count = 0;

    for (ds_list_node_t* ptr = vector_list->list.head; ptr; ptr=ptr->next)
    {
        int_dispatch_linked_t* dsp_node = container_of(ptr, int_dispatch_linked_t, topo);
        int st = dsp_node->dispatch.dispatcher(regs, dsp_node->dispatch.ctx);
        if (st == INT_STATUS_HANDLED) 
        {
            dsp_node->dispatch.ops->ack(dsp_node->dispatch.interrupt_num);
            handled_count++;
        }
    }
    return handled_count;
}

int_dispatch_blk_t* interrupt_vector_find_dispatcher(int vector, int_dispatcher_cb_t dispatcher)
{
    if (vector < int_vector_offset) return NULL;
    int_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    for (ds_list_node_t* ptr = vector_list->list.head; ptr; ptr = ptr->next)
    {
        int_dispatch_linked_t* dsp_node = container_of(ptr, int_dispatch_linked_t, topo);
        if (dsp_node->dispatch.dispatcher == dispatcher) return &dsp_node->dispatch;
    }
    return NULL;
}

int interrupt_vector_populate(int vector, int int_num, int_dispatcher_ops_t* ops, int_dispatcher_cb_t dispatcher, void* ctx)
{
    if (vector < int_vector_offset) return -1;
    int_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    if (!ops) return -1;
    if (!dispatcher) return -1;

    int_dispatch_blk_t* disp_blk = interrupt_vector_find_dispatcher(vector, dispatcher);
    if (disp_blk) return -1;

    int_dispatch_linked_t* disp_linked_blk = (int_dispatch_linked_t*)mmu_heap_alloc(sizeof(int_dispatch_linked_t*));
    if (!disp_linked_blk) return -1;

    disp_linked_blk->dispatch = (int_dispatch_blk_t){
        .dispatcher = dispatcher,
        .ctx = ctx,
        .interrupt_num = int_num,
        .ops = ops
    };
    ds_list_node_init(&disp_linked_blk->topo);

    int st = ds_list_push_back(&vector_list->list, &disp_linked_blk->topo);
    if (st < 0)
    {
        mmu_heap_free(disp_linked_blk);
        return -1;
    }

    vector_list->size++;
    interrupt_vector_set_lower_priority(vector_list);
    return 0;
}

void interrupt_vector_unlink_dispatcher(int vector, int_dispatcher_cb_t dispatcher)
{
    if (vector < int_vector_offset) return;
    int_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];
    if (vector_list->size == 0) return;

    int_dispatch_blk_t* dispatch_blk = interrupt_vector_find_dispatcher(vector, dispatcher);
    if (!dispatch_blk) return;

    int_dispatch_linked_t* node = container_of(dispatch_blk, int_dispatch_linked_t, dispatch);

    int st = ds_list_remove(&vector_list->list, &node->topo);
    if (st < 0 ) return;

    mmu_heap_free(node);

    dispatch_array[vector-int_vector_offset].size--;
    interrupt_vector_set_higher_priority(vector_list);
}

static void interrupt_vector_set_lower_priority(int_dispatch_blk_list_t* vector_list)
{
    if (!vector_list) return;

    int current_idx = vector_list->priority_id;
    int vector_offset = priority_list[current_idx];
    int_dispatch_blk_list_t* cur = &dispatch_array[vector_offset-int_vector_offset];

    for (int i=current_idx+1;i<int_vector_count;i++)
    {
        int next_vector_offset = priority_list[i];
        int_dispatch_blk_list_t* next = &dispatch_array[next_vector_offset-int_vector_offset];

        if (cur->size >= next->size)
        {
            swap(&priority_list[current_idx], &priority_list[i]);
            current_idx = i;
        }
    }

    vector_list->priority_id = current_idx;
}

static void interrupt_vector_set_higher_priority(int_dispatch_blk_list_t* vector_list)
{
    if (!vector_list) return;

    int current_idx = vector_list->priority_id;
    int vector_offset = priority_list[current_idx];
    int_dispatch_blk_list_t* cur = &dispatch_array[vector_offset-int_vector_offset];

    for (int i=current_idx-1;i>=0;i--)
    {
        int prev_vector_offset = priority_list[i];
        int_dispatch_blk_list_t* prev = &dispatch_array[prev_vector_offset-int_vector_offset];

        if (cur->size >= prev->size)
        {
            swap(&priority_list[current_idx], &priority_list[i]);
            current_idx = i;
        }
    }

    vector_list->priority_id = current_idx;
}