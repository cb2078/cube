#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <immintrin.h>
#include <limits.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define ABS(x) ({ typeof(x) a=(x); a>0 ? a : -a; })
#define ASSERT(cond)\
    (!(cond) ? fprintf(stderr, "%s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, #cond), DEBUGBREAK(), exit(1) : (void)0)
#define ERROR(...) (fprintf(stderr, "cube: " __VA_ARGS__), exit(1))
#define LENGTH(x) (long long)(sizeof(x)/sizeof(x[0]))
#define LOG(...) (VERBOSE ? fprintf(stderr, __VA_ARGS__) : (void)0)
#define MAX(x, y) ({ typeof(x) a=(x), b=(y); a>b ? a : b; })
#define MIN(x, y) ({ typeof(x) a=(x), b=(y); a<b ? a : b; })
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)

#ifdef DEBUG
#define DEBUGBREAK() ({ __asm__ volatile("int $0x03"); })
#define UNREACHABLE() (DEBUGBREAK(), __builtin_unreachable())
#else
#define DEBUGBREAK() (void)0
#define UNREACHABLE() __builtin_unreachable()
#endif

#ifdef DEBUG
static int VERBOSE = 1;
#else
static int VERBOSE = 0;
#endif

static int THREADS = 4;
static int NO_INPUT = 0;

#endif
