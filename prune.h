#pragma once

#include "coord.h"
#include "map.h"

#define USE_PREPASS 1
#define MAX_DEPTH 15

#define MAP_DEPTH 8
#define PRUNE_BASE (prune_base[EO_VARIANT])

// x*529>>15 is a fast way to calculate x/62 for x = 0..494
#define INSERT_ESLICE_GAPS(x) ((x)+((x)*529>>15)*2)
#define PRUNE_EXT_62(i) ((i)/choose[12][4]*512 + INSERT_ESLICE_GAPS((i)%choose[12][4]))
#define PRUNE_MIN_62(i) ((i)/62*64+62)

struct fill_prune_table_arg
{
    mtx_t *mutexes;
    int thread_id;
    int depth;
    struct map *map;
};

static void fill_sym_table(void);
static void fill_prune_table(void);

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
