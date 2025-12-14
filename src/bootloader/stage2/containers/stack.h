#pragma once
#include "../stdint.h"

#define STACK_BASE_SIZE 256

#define STACK(type) \
struct stack_t \
{ \
    usize capacity; \
    isize top; \
    type* ptr; \
}

#define STACK_INIT(_s, _c, _p) \
do { \
    (_s).capacity = (_c); \
    (_s).top = -1; \
    (_s).ptr = (_p); \
} while (0)

#define STACK_FINI(_s) \
do { \
    (_s).ptr = NULL; \
} while (0)

#define STACK_EMPTY(_s) ((_s).top < 0)

#define STACK_PUSH(_s, _type, _val) \
do { \
    if ((_s).top+1>=(_s).capacity) { \
        STACK_FINI((_s)); \
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