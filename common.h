#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(x) ({ typeof(x) a=(x); a>0 ? a : -a; })
#define ASSERT(cond) ((cond) ? (void)0 : UNREACHABLE())
#define ERROR(fmt, ...) (fprintf(stderr, "cube: " fmt, __VA_ARGS__), exit(1))
#define LENGTH(x) (long long)(sizeof(x)/sizeof(x[0]))
#define MAX(x, y) ({ typeof(x) a=(x), b=(y); a>b ? a : b; })
#define MIN(x, y) ({ typeof(x) a=(x), b=(y); a<b ? a : b; })
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)
#define UNREACHABLE() (debugbreak(), __builtin_unreachable())

static inline void debugbreak(void)
{
    __asm__ volatile("int $0x03");
}

#endif
