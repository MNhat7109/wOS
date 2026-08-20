#pragma once
#include <stdint.h>
#include <stddef.h>

#define DS_STATUS_SUCCESS 0
#define DS_STATUS_INVALID_INPUT 1

typedef signed long ssize_t;

#define container_of(_p, _t, _n) ({ \
    const void* p = (_p); \
    (p)? ((_t*)(((uint8_t*)(p)-offsetof(_t,_n)))) \
    : NULL; \
})
