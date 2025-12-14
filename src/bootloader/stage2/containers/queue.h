#pragma once
#include "../stdint.h"

#define QUEUE(type)    \
struct queue_t  \
{                      \
    usize capacity;    \
    isize front, rear; \
    type* ptr;         \
} 

#define QUEUE_INIT(_q, _c, _p) \
do { \
    (_q).capacity = (_c); \
    (_q).front = 0; (_q).rear = -1; \
    (_q).ptr = (_p); \
} while (0)

#define QUEUE_FINI(_q) \
do { \
    (_q).ptr = NULL; \
} while (0)

#define QUEUE_EMPTY(_q) ((_q).front > (_q).rear)

#define QUEUE_PUSH(_q, _type, _val) \
do { \
    if ((_q).rear+1>=(_q).capacity) { \
        QUEUE_FINI((_q)); \
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