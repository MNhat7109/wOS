#include <stack.h>
#include <list.h>

void ds_stack_init(ds_stack_t* stack)
{
    stack->size = 0;
    ds_list_init(&stack->list);
}

void ds_stack_node_init(ds_stack_node_t* node)
{
    if (!node) return;

    ds_list_node_init(node);
}

int ds_stack_push(ds_stack_t* stack, ds_stack_node_t* node)
{
    if (!stack) return DS_STATUS_INVALID_INPUT;
    if (!node) return DS_STATUS_INVALID_INPUT;

    int status = ds_list_push_front(&stack->list, node);
    if (status < DS_STATUS_SUCCESS) return status;

    stack->size++; return DS_STATUS_SUCCESS;
}

int ds_stack_pop(ds_stack_t* stack)
{
    if (!stack) return DS_STATUS_INVALID_INPUT;
    if (stack->size == DS_STATUS_SUCCESS) return DS_STATUS_INVALID_INPUT;

    int status = ds_list_remove(&stack->list, stack->list.head);
    if (status < DS_STATUS_SUCCESS) return status;

    stack->size--;
    return DS_STATUS_SUCCESS;
}

ds_stack_node_t* ds_stack_top(ds_stack_t* stack)
{
    if (!stack) return NULL;
    return stack->list.head;
}