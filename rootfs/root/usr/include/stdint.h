#pragma once

#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void*)0)
#endif

#ifdef USE_KERNEL_TYPES
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int      u32;
typedef unsigned long long u64;

typedef signed char      i8;
typedef signed short     i16;
typedef signed int      i32;
typedef signed long long i64;

#ifdef __x86_64__
typedef unsigned long usize;
typedef signed long isize;
typedef unsigned long uptr;
typedef signed long iptr;
#else
typedef unsigned int usize;
typedef signed int isize;
typedef unsigned int uptr;
typedef signed int iptr;
#endif

#endif

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char      int8_t;
typedef signed short     int16_t;
typedef signed int       int32_t;
typedef signed long long int64_t;

#ifdef __x86_64__
typedef unsigned long size_t;
typedef signed long ssize_t;
typedef unsigned long uintptr_t;
typedef signed long intptr_t;
#else
typedef unsigned int size_t;
typedef signed int ssize_t;
typedef unsigned int uintptr_t;
typedef signed int intptr_t;
#endif
