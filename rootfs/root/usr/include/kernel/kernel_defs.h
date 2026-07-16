#pragma once

#ifndef __i386__
#define USER_BASE 0
#define USER_END 0xBFFFFFFF
#define KERNEL_BASE 0xC0000000
#define KERNEL_END 0xFFFFFFFF
#else
#define USER_BASE 0
#define USER_END 0xBFFFFFFF
#define KERNEL_BASE 0xC0000000
#define KERNEL_END 0xFFFFFFFF
#endif