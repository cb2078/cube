#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT assert // TODO use custom assert
#define ABS(x) ({ typeof(x) a=(x); a>0 ? a : -1; })
#define LENGTH(x) (long long)(sizeof(x)/sizeof(x[0]))
#define MAX(x, y) ({ typeof(x) a=(x), b=(y); a>b ? a : b; })
#define MIN(x, y) ({ typeof(x) a=(x), b=(y); a<b ? a : b; })
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)
#define UNREACHABLE() (assert(0 && "unreachable"), exit(1)) // TODO use custom unreachable too

#endif
