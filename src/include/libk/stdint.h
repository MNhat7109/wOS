#pragma once

#define NULL 0

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int      u32;
typedef unsigned long long u64;

typedef signed char      i8;
typedef signed short     i16;
typedef signed int      i32;
typedef signed long long i64;

#ifdef __x86_64__
typedef unsigned long long usize;
typedef signed long long isize;
#else
typedef unsigned long usize;
typedef signed long isize;
#endif