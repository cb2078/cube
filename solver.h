#ifndef SOLVER_H
#define SOLVER_H

#include "coord.h"
#include "cube.h"

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
    int thread_id;
    struct queue_node *queue;
    int path[20];
    int start_depth;
    int max_depth;
    int *done;
};

static void optimal(cube_t, int *, int *);

static inline int h_phase1(cube_t x)
{
    long long i = coord_phase1.get(x);
    long long j = PRUNE_EXT_62(i);
    long long k = PRUNE_MIN_62(i);
    int r = table_get(coord_phase1.table, 2, j);
    return r ? r+PRUNE_BASE : table_get(coord_phase1.table, 4, k/2);
}

#endif
