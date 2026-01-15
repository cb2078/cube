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
    [0] = 7,
    [1] = 7,
    [2] = 7,
    [3] = 7,
    [4] = 8,
    [5] = 8,
    [6] = 8,
    [7] = 8,
    [8] = 9,
    [9] = 9,
    [10] = 9,
    [11] = 9,
};

#endif
