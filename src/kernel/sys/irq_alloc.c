#include <kernel/irq.h>
#include <kernel/arch/i686/irq.h>
#include <kernel/arch/i686/state.h>
#include <ds/list.h>
#include <kernel/mmu_heap.h>
#include <kernel/debug.h>

static void swap(void* a, void* b) { void* _m = (a);  a = (b); b = (_m); }

#define MAX_DISPATCHER_COUNT 4
typedef struct irq_dispatch_blk_list_t
{
    usize size;
    ds_list_t list;
} irq_dispatch_blk_list_t;

extern struct irq_data_t irq_data;

struct irq_dispatched_linked_t
{
    irq_dispatch_blk_t dispatch;
    ds_list_node_t topo;
};

static int int_vector_count, int_vector_offset;

static irq_dispatch_blk_list_t* dispatch_array;

static int* priority_list;

int irq_vector_alloc_init(int vector_offset, int vector_count)
{
    kdebugf(DEBUG_INFO, MODULE_IRQ, "Setting up vector allocator...\n");
    int_vector_offset = vector_offset;
    int_vector_count = vector_count;

    // Set up an array of linked dispatches
    dispatch_array = (irq_dispatch_blk_list_t*)mmu_heap_alloc(int_vector_count*sizeof(*dispatch_array));
    if (!dispatch_array) return -1;

    // Set up a priority list to keep track of the unused/free vectors
    priority_list = (int*)mmu_heap_alloc(int_vector_count*sizeof(*priority_list));
    if (!priority_list) return -1;
    
    for (int i=0;i<int_vector_count;i++)
    {
        dispatch_array[i].size = 0;
        ds_list_init(&dispatch_array[i].list);
        priority_list[i] = int_vector_offset+i;
    }
    kdebugf(DEBUG_INFO, MODULE_IRQ, "%d vector(s) available starting from offset 0x%x\n", int_vector_count, int_vector_offset);
}

void irq_vector_rearrange(int current_idx)
{
    int vector_offset = priority_list[current_idx];
    irq_dispatch_blk_list_t* cur = &dispatch_array[vector_offset-int_vector_offset];

    for (int i=current_idx+1;i<int_vector_count;i++)
    {
        int next_vector_offset = priority_list[i];
        irq_dispatch_blk_list_t* next = &dispatch_array[next_vector_offset-int_vector_offset];

        if (cur->size >= next->size)
        {
            swap(&priority_list[current_idx], &priority_list[i]);
            current_idx = i;
        }
    }
}

u8 irq_vector_alloc()
{
    int current_vector_offset = priority_list[0];
    irq_dispatch_blk_list_t* list = &dispatch_array[current_vector_offset-int_vector_count];
    if (list->size >= MAX_DISPATCHER_COUNT) return 0;
    return current_vector_offset;
}

irq_dispatch_blk_t* irq_vector_find(int type, u8 vector, irq_dispatcher_cb_t dispatcher)
{
    if (vector < int_vector_offset) return NULL;
    irq_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    for (ds_list_node_t* ptr = vector_list->list.head; ptr; ptr=ptr->next)
    {
        struct irq_dispatched_linked_t* dsp_node = container_of(ptr, struct irq_dispatched_linked_t, topo);
        if (dsp_node->dispatch.type == type && dsp_node->dispatch.dispatcher == dispatcher) return &dsp_node->dispatch;    
    }
    return NULL;
}

int irq_vector_follow_chain(u8 vector, register_state_t* regs)
{
    if (vector < int_vector_offset) return -1;
    irq_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    int handled_count = 0;

    for (ds_list_node_t* ptr = vector_list->list.head; ptr; ptr=ptr->next)
    {
        struct irq_dispatched_linked_t* dsp_node = container_of(ptr, struct irq_dispatched_linked_t, topo);
        int st = dsp_node->dispatch.dispatcher(regs, dsp_node->dispatch.ctx);
        if (st == IRQ_STATUS_HANDLED) 
        {
            handled_count++;
        }
    }
    return handled_count;
}

int irq_vector_populate(int type, u8 vector, int irq, irq_dispatcher_cb_t dispatcher, void* ctx)
{
    if (vector < int_vector_offset) return -1;
    irq_dispatch_blk_list_t* vector_list = &dispatch_array[vector-int_vector_offset];

    struct irq_dispatched_linked_t* blk = (struct irq_dispatched_linked_t*)mmu_heap_alloc(sizeof(struct irq_dispatched_linked_t));
    if (!blk) return -1;

    blk->dispatch = (irq_dispatch_blk_t){
        .dispatcher = dispatcher,
        .ctx = ctx,
        .irq = irq,
        .type = type
    };
    ds_list_node_init(&blk->topo);

    int st = ds_list_push_back(&vector_list->list, &blk->topo);
    if (st < 0)
    {
        mmu_heap_free(blk);
        return -1;
    }

    vector_list->size++;
    irq_vector_rearrange(0);
    return 0;
}

void irq_vector_free(int type, u8 vector, irq_dispatcher_cb_t dispatcher)
{
    // Find dispatcher in vector
    irq_dispatch_blk_t* disp_blk = irq_vector_find(type, vector, dispatcher);
    if (!disp_blk) return;

    // Free stuff
    struct irq_dispatched_linked_t* linked_node = container_of(disp_blk, struct irq_dispatched_linked_t, dispatch);

    int st = ds_list_remove(&dispatch_array[vector-int_vector_offset].list, &linked_node->topo);
    if (st < 0) return;
    mmu_heap_free(linked_node);
}