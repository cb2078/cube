#ifndef SOLVER_H
#define SOLVER_H

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

static inline int in_H(cube_t x)
{
    // NOTE since this is checking that the coordinate is 0, we do not need
    // use the symmetric coordinate and instead just check that both raw
    // ones are 0. This might be faster sisnce it isn't using the RAM for
    // handling the sym coordinate.
    //
    // something like: get_co_csep(x) == 0 && get_esep(x) == 0
    return coord_phase1.get(x) == 0;
}

static inline int h_phase1(cube_t x)
{
    long long i = coord_phase1.get(x);
    long long j = PRUNE_EXT_62(i);
    long long k = PRUNE_MIN_62(i);
    int r = table_get(coord_phase1.table, 2, j);
    return r ? r+PRUNE_BASE : table_get(coord_phase1.table, 4, k/2);
}

static inline int h_phase2(cube_t x)
{
    ASSERT(in_H(x));
    return MIN(table_get(coord_phase2.table, coord_phase2.bits, coord_phase2.get(x)), MAX_DEPTH);
}

#endif
