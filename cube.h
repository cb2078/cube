#ifndef CUBE_H
#define CUBE_H

#define NUM_CORNERS 8
#define NUM_EDGES 12

typedef union
{
    struct
    {
        char urf, ulb, drb, dlf;
        char urb, ulf, drl, dlb;
        char rf, rb, lf, lb;
        char uf, ub, df, db;
        char ur, ul, dr, dl;
    };
    struct
    {
        char corners[NUM_CORNERS];
        char edges[NUM_EDGES];
    };
    struct
    {
        char cubies[NUM_CORNERS+NUM_EDGES];
    };
    struct
    {
        char urf_tetrad[4];
        char urb_tetrad[4];
        char ud_slice[4];
        char rl_slice[4];
        char fb_slice[4];
    };
} cube;

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
enum tetrad
{
    TETRAD_URF,
    TETRAD_URB,
};

enum slice
{
    SLICE_UD,
    SLICE_RL,
    SLICE_FB,
};

static cube new_cube(void);
static void print_cube(cube);
static int cube_eq(cube, cube);
static cube compose(cube, cube);
static cube compose_3(cube, cube, cube);
static cube apply_sym(cube, int);
static cube apply_move(cube, int);
static cube apply_moves(cube, int *, int);
static cube invert_co(cube);

static int get_co(cube);
static int get_eo(cube);
static void set_co(cube *, long long);
static void set_eo(cube *, long long);
static int get_tetrad_twist(cube);
static void set_tetrad_twist(cube *, int);

#endif
