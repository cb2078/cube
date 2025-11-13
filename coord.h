#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "moves.h"
#include "table.h"
#include "util.h"

struct coord
{
    char name[256];
    enum
    {
        RAW,
        COMP,
        SYM_COMP,
    } type;

    // raw
    enum
    {
        ORIENTATION,
        PERMUTATION,
        PARTIAL_PERMUTATION,
        COMBINATION,
        CUSTIOM,
    } indexer;
    enum
    {
        CORNERS,
        URF_TETRAD,
        URB_TETRAD,
        EDGES,
        UD_SLICE,
        RL_SLICE,
        FB_SLICE,
    } pieces;
    int offset;
    int length;
    long long max;

    // composite
    int count;
    struct coord *coords;

    // symmetric
    int num_syms; // TODO use a mask instead
    int classes;
    long long *rep;
    struct
    {
        unsigned class: 16;
        unsigned sym: 16;
    } *coord_info;
    long long *self_syms;

    // pruning table
    table *table;
    int move_mask;
};

////////////////////////////////////////////////////////////////////////////////

static const struct coord coord_flip =
{
    .type = RAW,
    .indexer = ORIENTATION,
    .pieces = EDGES,
    .max = pow2[NUM_EDGES-1],
};

static const struct coord coord_twist =
{
    .type = RAW,
    .indexer = ORIENTATION,
    .pieces = CORNERS,
    .max = pow3[NUM_CORNERS-1],
};

static const struct coord coord_ud_slice =
{
    .type = RAW,
    .indexer = COMBINATION,
    .pieces = EDGES,
    .length = 4,
    .max = choose[12][4],
};

static const struct coord coord_rl_slice =
{
    .type = RAW,
    .indexer = COMBINATION,
    .pieces = EDGES,
    .length = 4,
    .offset = 4,
    .max = choose[8][4],
};

static const struct coord coord_corner_sep =
{
    .type = RAW,
    .indexer = COMBINATION,
    .pieces = CORNERS,
    .length = 4,
    .max = choose[8][4],
};

static const struct coord coord_flip_ud_slice =
{
    .type = COMP,
    .count = 2,
    .coords = (struct coord []){coord_flip, coord_ud_slice},
    .max = 1013760,
};

static const struct coord coord_corner_perm =
{
    .type = RAW,
    .indexer = PERMUTATION,
    .pieces = CORNERS,
    .max = fact[8],
};

static const struct coord coord_ud_edge_perm =
{
    .type = RAW,
    .indexer = PERMUTATION,
    .pieces = EDGES,
    .offset = 4,
    .max = fact[8],
};

static const struct coord coord_twist_corner_sep =
{
    .type = COMP,
    .count = 2,
    .coords = (struct coord []){coord_twist, coord_corner_sep},
    .max = 153090,
};

////////////////////////////////////////////////////////////////////////////////

static struct coord coord_stage1 =
{
    .name = "stage1",
    .type = COMP,
    .count = 1,
    .coords = (struct coord []){coord_flip},
    .max = 2048,
};

static struct coord coord_stage2 =
{
    .name = "stage2",
    .type = COMP,
    .count = 2,
    .coords = (struct coord []){coord_ud_slice, coord_twist},
    .move_mask = EO_MASK,
    .max = 1082565,
};

static struct coord coord_stage3 =
{
    .name = "stage3",
    .type = COMP,
    .count = 2,
    .coords = (struct coord []){coord_rl_slice, coord_corner_sep},
    .move_mask = DR_MASK,
    .max = 4900,
};

static struct coord coord_stage4 =
{
    .name = "stage4",
    .type = COMP,
    .count = 5,
    .coords = (struct coord [])
    {
        {
            .type = RAW,
            .indexer = PERMUTATION,
            .pieces = URF_TETRAD,
            .max = fact[4],
        },
        {
            .type = RAW,
            .indexer = PARTIAL_PERMUTATION,
            .pieces = URB_TETRAD,
            .length = 1,
            .max = pick[4][1],
        },
        {
            .type = RAW,
            .indexer = PERMUTATION,
            .pieces = UD_SLICE,
            .max = fact[4],
        },
        {
            .type = RAW,
            .indexer = PERMUTATION,
            .pieces = RL_SLICE,
            .max = fact[4],
        },
        {
            .type = RAW,
            .indexer = PARTIAL_PERMUTATION,
            .pieces = FB_SLICE,
            .length = 2,
            .max = pick[4][2],
        },
    },
    .move_mask = HTR_MASK,
    .max = 663552,
};

static struct coord coord_phase1 =
{
    .name = "phase1",
    .type = SYM_COMP,
    .classes = 64430,
    .num_syms = 16,
    .coords = (struct coord []){coord_flip_ud_slice, coord_twist},
    .max = 140908410,
};

static struct coord coord_phase2 =
{
    .name = "phase2",
    .type = SYM_COMP,
    .classes = 2768,
    .num_syms = 16,
    .coords = (struct coord[]){coord_corner_perm, coord_ud_edge_perm},
    .move_mask = DR_MASK,
    .max = 111605760,
};

static struct coord coord_optimal =
{
    .name = "optimal",
    .type = SYM_COMP,
    .classes = 64430,
    .num_syms = 16,
    .coords = (struct coord[]){coord_flip_ud_slice, coord_twist_corner_sep},
    .max = 9863588700,
};

////////////////////////////////////////////////////////////////////////////////

static inline long long coord_get(struct coord *, cube);
static inline cube coord_set(struct coord *, long long);

static int coord_read(struct coord *);
static int coord_write(struct coord *);

// static void coord_init_sym(struct coord *);
static void init_sym(struct coord *);

#endif
