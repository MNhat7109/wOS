#pragma once
#include <libk/stdint.h>
#include <libk/stdlib.h>

#define QUEUE_BASE_SIZE 256

#define QUEUE(type)    \
struct queue_t  \
{                      \
    usize capacity;    \
    isize front, rear; \
    type* ptr;         \
} 

#define QUEUE_INIT(_q) \
do { \
    (_q).capacity = QUEUE_BASE_SIZE; \
    (_q).front = 0; (_q).rear = -1; \
    (_q).ptr = kmalloc(sizeof(*(_q).ptr)*QUEUE_BASE_SIZE); \
} while (0)

#define QUEUE_FINI(_q) \
do { \
    kfree((_q).ptr); \
    (_q).ptr = NULL; \
} while (0)

#define QUEUE_EMPTY(_q) ((_q).front > (_q).rear)

#define QUEUE_PUSH(_q, _type, _val) \
do { \
    if ((_q).rear+1>=(_q).capacity) { \
        (_q).capacity <<= 1;        \
        _type* new_q_ptr = krealloc((_q).ptr, sizeof(*(_q).ptr)*(_q).capacity); \
        if (!new_q_ptr) { \
            QUEUE_FINI((_q)); \
        } \
        else { (_q).ptr = new_q_ptr; } \
    } \
    if ((_q).ptr) { \
        (_q).ptr[++(_q).rear] = (_val); \
    } \
} while (0)

#define QUEUE_POP(_q) \
do { \
    if (!QUEUE_EMPTY((_q))) { \
        (_q).front++; \
    } \
} while (0)

#define QUEUE_FRONT(_q, _type) (((_q).ptr)?(_q).ptr[(_q).front]:(_type){0})
#define QUEUE_REAR(_q, _type) (((_q).ptr)?(_q).ptr[(_q).rear]:(_type){0})