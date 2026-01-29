#ifndef PRUNE_H
#define PRUNE_H

#include "coord.h"
#include "map.h"

#define MAP_DEPTH 8
#define PRUNE_BASE (prune_base[EO_VARIANT])

#define PRUNE_MIN_62(i) ((i)/64*64+62)

struct init_prune_table_arg
{
    mtx_t *mutexes;
    int thread_id;
    struct coord *c;
    int depth;
    struct map *map;
};

static void init_sym(struct sym_coord *c);
static void init_prune_table(struct coord *c);

static int prune_base[12] =
{
    [0] = 8,
    [1] = 8,
    [2] = 8,
    [3] = 8,
    [4] = 9,
    [5] = 9,
    [6] = 9,
    [7] = 9,
    [8] = 10,
    [9] = 10,
    [10] = 10,
    [11] = 10,
};

#endif
