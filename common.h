#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define ABS(x) ({ typeof(x) a=(x); a>0 ? a : -a; })
#define ASSERT(cond)\
    (!(cond) ? fprintf(stderr, "%s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, #cond), UNREACHABLE() : (void)0)
#define ERROR(...) (fprintf(stderr, "cube: " __VA_ARGS__), exit(1))
#define LENGTH(x) (long long)(sizeof(x)/sizeof(x[0]))
#define MAX(x, y) ({ typeof(x) a=(x), b=(y); a>b ? a : b; })
#define MIN(x, y) ({ typeof(x) a=(x), b=(y); a<b ? a : b; })
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)

#ifdef DEBUG
#define UNREACHABLE() (debugbreak(), __builtin_unreachable())
#else
#define UNREACHABLE() __builtin_unreachable()
#endif

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define LOG(...) (void)0
#endif

static inline void debugbreak(void)
{
    __asm__ volatile("int $0x03");
}

#endif
