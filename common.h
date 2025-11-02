#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(x) ((x)<0 ? -(x) : x)
#define LENGTH(x) (long signed int)(sizeof(x)/sizeof(x[0]))
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)
#define UNREACHABLE() assert(0 && "unreachable")

#endif
