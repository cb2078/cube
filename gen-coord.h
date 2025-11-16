#include "cube.h"
#include "util.h"

struct coord
{
    char *name;
    char *move_mask;
    enum
    {
        TETRAD_TWIST,
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
    int ignore_orientation;

    // symmetric
    int num_syms;
    int classes;

    // composite
    int count;
    const struct coord *coords;
};

static const struct coord flip =
{
    .name = "flip",
    .type = RAW,
    .indexer = ORIENTATION,
    .pieces = EDGES,
    .max = pow2[11],
};

static const struct coord twist =
{
    .type = RAW,
    .indexer = ORIENTATION,
    .pieces = CORNERS,
    .max = pow3[7],
};

static const struct coord ud_slice =
{
    .type = RAW,
    .indexer = COMBINATION,
    .pieces = EDGES,
    .length = 4,
    .max = choose[12][4],
};

static const struct coord flip_ud_slice =
{
    .name = "flip_ud_slice",
    .type = COMP,
    .count = 2,
    .coords = (struct coord [])
    {
        flip,
        {
            .type = RAW,
            .indexer = COMBINATION,
            .pieces = EDGES,
            .length = 4,
            .max = choose[12][4],
            .ignore_orientation = 1,
        },
    },
    .ignore_orientation = 1,
};

static const struct coord corner_perm =
{
    .name = "corner_perm",
    .type = RAW,
    .indexer = PERMUTATION,
    .pieces = CORNERS,
    .max = fact[8],
};

static const struct coord ud_edge_perm =
{
    .type = RAW,
    .indexer = PERMUTATION,
    .pieces = EDGES,
    .offset = 4,
    .max = fact[8],
};

////////////////////////////////////////////////////////////////////////////////

static const struct coord thistlethwaite[] =
{
    flip,
    {
        .name = "stage2",
        .type = COMP,
        .count = 2,
        .coords = (struct coord []){twist, ud_slice},
        .move_mask = "EO_MASK",
    },
    {
        .name = "stage3",
        .type = COMP,
        .count = 3,
        .coords = (struct coord [])
        {
            {
                .type = RAW,
                .indexer = COMBINATION,
                .pieces = EDGES,
                .offset = 4,
                .length = 4,
                .max = choose[8][4],
            },
            {
                .type = TETRAD_TWIST,
            },
            {
                .type = RAW,
                .indexer = COMBINATION,
                .pieces = CORNERS,
                .length = 4,
                .max = choose[8][4],
            },
        },
        .move_mask = "DR_MASK",
    },
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
        .move_mask = "HTR_MASK",
    },
};

static const struct coord kociemba[] =
{
    {
        .name = "phase1",
        .type = SYM_COMP,
        .num_syms = 16,
        .classes = 64430,
        .coords = (struct coord []){flip_ud_slice, twist},
    },
    {
        .name = "phase2",
        .type = SYM_COMP,
        .num_syms = 16,
        .classes = 2768,
        .coords = (struct coord []){corner_perm, ud_edge_perm},
        .move_mask = "DR_MASK",
    },
};

static const struct coord optimal[] =
{
    {
        .name = "optimal",
        .type = SYM_COMP,
        .num_syms = 16,
        .classes = 788,
        .coords = (struct coord [])
        {
            {
                .name = "ud_slice_sorted",
                .type = RAW,
                .indexer = PARTIAL_PERMUTATION,
                .pieces = EDGES,
                .length = 4,
                .max = pick[12][4],
                .ignore_orientation = 1,
            },
            {
                .type = COMP,
                .count = 2,
                .coords = (struct coord []){flip, twist},
            },
        }
    },
};
