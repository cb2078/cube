#pragma once

#include "coord.h"
#include "cube.h"

#define QUEUE_LENGTH 43254
#define QUEUE_DEPTH 4

struct queue_node
{
    cube_t cube;
    unsigned path;
};

struct search_node
{
    cube_t cube;
    int move;
    int depth;
};

struct search_arg
{
    int *start;
    struct queue_node *queue;
    int *path;
    int depth;
    int *done;
};

static void optimal(cube_t, int *, int *);

static inline int h(cube_t x)
{
    long long i = get_coord(x, EO_VARIANT);
    long long j = PRUNE_EXT_62(i);
    long long k = PRUNE_MIN_62(i);
    int r = table_get(coord.table, 2, j);
    return r ? r+PRUNE_BASE : table_get(coord.table, 4, k/2);
}
