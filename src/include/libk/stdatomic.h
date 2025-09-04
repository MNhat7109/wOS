#pragma once
#include <libk/stdint.h>

#define ATOMIC_TYPE(t) \
    struct atomic_##t { t value __attribute__((aligned(sizeof(t)))); }

#define ATOMIC_STORE(_n, _v) (__atomic_store_n(&((_n).value), (_v), __ATOMIC_SEQ_CST))
#define ATOMIC_LOAD(_n) (__atomic_load_n(&((_n).value), __ATOMIC_SEQ_CST))
#define ATOMIC_ADD(_n, _v) (__atomic_add_fetch(&((_n).value), (_v), __ATOMIC_SEQ_CST))
#define ATOMIC_SUB(_n, _v) (__atomic_sub_fetch(&((_n).value), (_v), __ATOMIC_SEQ_CST))