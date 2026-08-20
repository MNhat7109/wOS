#include <queue.h>
#include <list.h>

void ds_queue_init(ds_queue_t* queue)
{
    queue->size = 0;
    ds_list_init(&queue->list);
}

void ds_queue_node_init(ds_queue_node_t* node)
{
    ds_list_node_init(node);
}

int ds_queue_push(ds_queue_t* queue, ds_queue_node_t* node)
{
    if (!queue) return DS_STATUS_INVALID_INPUT;
    if (!node) return DS_STATUS_INVALID_INPUT;

    int status = ds_list_push_back(&queue->list, node);
    if (status < DS_STATUS_SUCCESS) return status;

    queue->size++;
    return DS_STATUS_SUCCESS;
}

int ds_queue_pop(ds_queue_t* queue)
{
    if (!queue) return DS_STATUS_INVALID_INPUT;
    if (queue->size == DS_STATUS_SUCCESS) return DS_STATUS_INVALID_INPUT;

    int status = ds_list_remove(&queue->list, queue->list.head);
    if (status < DS_STATUS_SUCCESS) return status;

    queue->size--;
    return DS_STATUS_SUCCESS;
}

ds_queue_node_t* ds_queue_front(ds_queue_t* queue)
{
    if (!queue) return NULL;
    return queue->list.head;
}

ds_queue_node_t* ds_queue_back(ds_queue_t* queue)
{
    if (!queue) return NULL;
    return queue->list.tail;
}