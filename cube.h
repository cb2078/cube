#ifndef CUBE_H
#define CUBE_H

#define NUM_CORNERS 8
#define NUM_EDGES 12

#define CUBE(...) (cube_t){.cubies={__VA_ARGS__}}

typedef union
{
    struct
    {
        char corners[NUM_CORNERS];
        char edges[NUM_EDGES];
    };
    struct
    {
        char cubies[NUM_CORNERS+NUM_EDGES];
    };
} cube_t;

enum cubies
{
    URF, ULB, DRB, DLF,
    URB, ULF, DRF, DLB,
    RF,  RB,  LF,  LB,
    UF,  UB,  DF,  DB,
    UR,  UL,  DR,  DL,
};

static char *cubie_str[] =
{
    "URF", "ULB", "DRB", "DLF",
    "URB", "ULF", "DRF", "DLB",
    "RF",  "RB",  "LF",  "LB",
    "UF",  "UB",  "DF",  "DB",
    "UR",  "UL",  "DR",  "DL",
};

static cube_t new_cube(void);
static void print_cube(cube_t);
static int cube_eq(cube_t, cube_t);
static cube_t compose(cube_t, cube_t);
static cube_t compose_3(cube_t, cube_t, cube_t);
static cube_t apply_sym(cube_t, int);
static cube_t apply_move(cube_t, int);
static cube_t apply_moves(cube_t, int *, int);

static long long get_flip(cube_t);
static cube_t set_flip(long long);
static long long get_twist(cube_t);
static cube_t set_twist(long long);
static long long get_corner_sep(cube_t);
static cube_t set_corner_sep(long long);
static long long get_edge_sep(cube_t);
static cube_t set_edge_sep(long long);

static cube_t invert_co(cube_t);

#endif
