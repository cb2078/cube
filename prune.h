#ifndef PRUNE_H
#define PRUNE_H

#include "coord.h"
#include "map.h"

#define MAP_DEPTH 8

struct init_prune_table_arg
{
    mtx_t *mutexes;
    struct coord *c;
    int depth;
    int backsearch;
    long long start;
    long long end;
    struct map *map;
    _Atomic int done;
};

static void init_sym(struct sym_coord *c);
static void init_prune_table(struct coord *c);

#endif
