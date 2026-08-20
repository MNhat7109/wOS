#include <list.h>

typedef enum
{
    DS_VERIFY_VACANT = 0,
    DS_VERIFY_OWNED = 1,
    DS_VERIFY_DIFF_OWNER = 2,
} ds_list_verify_status;

int ds_list_verify_node(ds_list_t* list, ds_list_node_t* node)
{
    int status = DS_VERIFY_VACANT;
    if (!node->list) return status; 

    status |= DS_VERIFY_OWNED;
    if (node->list != list) status |= DS_VERIFY_DIFF_OWNER;

    return status;
}

void ds_list_init(ds_list_t* list)
{
    list->head = NULL;
    list->tail = NULL;
}

void ds_list_node_init(ds_list_node_t* node)
{
    node->list = NULL;
    node->prev = node->next = NULL;
}

int ds_list_push_front(ds_list_t* list, ds_list_node_t* node)
{
    if (!node) return DS_STATUS_INVALID_INPUT;
    if (!list) return DS_STATUS_INVALID_INPUT;
    int status = ds_list_verify_node(list, node);
    if (status & DS_VERIFY_OWNED) return DS_STATUS_INVALID_INPUT;

    if (list->head) list->head->prev = node;
    node->next = list->head;
    list->head = node;
    if (!list->tail) list->tail = node;

    node->list = list;
    return DS_STATUS_SUCCESS;
}

int ds_list_push_back(ds_list_t* list, ds_list_node_t* node)
{
    if (!node) return DS_STATUS_INVALID_INPUT;
    if (!list) return DS_STATUS_INVALID_INPUT;
    int status = ds_list_verify_node(list, node);
    if (status & DS_VERIFY_OWNED) return DS_STATUS_INVALID_INPUT;

    if (list->tail) list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
    if (!list->head) list->head = node;

    node->list = list;
    return DS_STATUS_SUCCESS;
}

int ds_list_insert_back_of(ds_list_t* list, ds_list_node_t* pos, ds_list_node_t* node)
{
    if (!list) return DS_STATUS_INVALID_INPUT;
    if (!pos) return DS_STATUS_INVALID_INPUT;
    if (!node) return DS_STATUS_INVALID_INPUT;
    int status = ds_list_verify_node(list, pos);
    if (status == DS_VERIFY_VACANT || (status & DS_VERIFY_DIFF_OWNER)) return DS_STATUS_INVALID_INPUT;

    status = ds_list_verify_node(list, node);
    if (status & DS_VERIFY_OWNED) return DS_STATUS_INVALID_INPUT;

    if (pos->next)
    {
        node->next = pos->next;
        node->next->prev = node;
    }
    else list->tail = node;

    pos->next = node;
    node->prev = pos;

    node->list = list;
    return DS_STATUS_SUCCESS;
}

int ds_list_insert_front_of(ds_list_t* list, ds_list_node_t* pos, ds_list_node_t* node)
{
    if (!list) return DS_STATUS_INVALID_INPUT;
    if (!pos) return DS_STATUS_INVALID_INPUT;
    if (!node) return DS_STATUS_INVALID_INPUT;
    int status = ds_list_verify_node(list, pos);
    if (status == DS_VERIFY_VACANT || (status & DS_VERIFY_DIFF_OWNER)) return DS_STATUS_INVALID_INPUT;

    status = ds_list_verify_node(list, node);
    if (status & DS_VERIFY_OWNED) return DS_STATUS_INVALID_INPUT;

    if (pos->prev)
    {
        node->prev = pos->prev;
        node->prev->next = node;
    }
    else list->head = node;

    pos->prev = node;
    node->next = pos;

    node->list = list;
    return DS_STATUS_SUCCESS;
}

int ds_list_remove(ds_list_t* list, ds_list_node_t* node)
{
    if (!list) return DS_STATUS_INVALID_INPUT;
    if (!node) return DS_STATUS_INVALID_INPUT;

    int status = ds_list_verify_node(list, node);
    if (status == DS_VERIFY_VACANT) return DS_STATUS_INVALID_INPUT;
    if (status & DS_VERIFY_DIFF_OWNER) return DS_STATUS_INVALID_INPUT;

    if (node->prev) node->prev->next = node->next;
    else list->head = node->next;

    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;

    node->prev = node->next = NULL;
    node->list = NULL;
    return DS_STATUS_SUCCESS;
}