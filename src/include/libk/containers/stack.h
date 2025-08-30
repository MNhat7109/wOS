#pragma once
#include <libk/stdint.h>
#include <libk/stdlib.h>

#define STACK_BASE_SIZE 256

#define STACK(type) \
struct stack_t \
{ \
    usize capacity; \
    isize top; \
    type* ptr; \
}

#define STACK_INIT(_s) \
do { \
    (_s).capacity = STACK_BASE_SIZE; \
    (_s).top = -1; \
    (_s).ptr = kmalloc(sizeof(*(_s).ptr)*(STACK_BASE_SIZE)); \
} while (0)

#define STACK_FINI(_s) \
do { \
    kfree((_s).ptr); \
    (_s).ptr = NULL; \
} while (0)

#define STACK_EMPTY(_s) ((_s).top < 0)

#define STACK_PUSH(_s, _type, _val) \
do { \
    if ((_s).top+1>=(_s).capacity) { \
        (_s).capacity <<= 1; \
        _type* new_s_ptr = krealloc((_s).ptr, sizeof(*(_s).ptr)*(_s).capacity); \
        if (!new_s_ptr) { \
            STACK_FINI((_s)); \
        } \
        else { (_s).ptr = new_s_ptr; } \
    } \
    if ((_s).ptr) { \
        (_s).ptr[++(_s).top] = (_val); \
    } \
} while (0)

#define STACK_POP(_s) \
do { \
    if (!STACK_EMPTY((_s))) { \
        (_s).top--; \
    } \
} while (0)

#define STACK_TOP(_s, _type) (((_s).ptr)?(_s).ptr[(_s).top]:(_type){0})