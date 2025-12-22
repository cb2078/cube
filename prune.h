#ifndef PRUNE_H
#define PRUNE_H

#include "coord.h"
#include "map.h"

#define MAP_DEPTH 8
#define PRUNE_BASE 8

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

#endif
