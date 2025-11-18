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

static cube_t new_cube(void);
static void print_cube(cube_t);
static int cube_eq(cube_t, cube_t);
static cube_t compose(cube_t, cube_t);
static cube_t compose_3(cube_t, cube_t, cube_t);
static cube_t apply_sym(cube_t, int);
static cube_t apply_move(cube_t, int);
static cube_t apply_moves(cube_t, int *, int);
static cube_t invert_co(cube_t);

static long long get_co(cube_t);
static long long get_eo(cube_t);
static void set_co(cube_t *, long long);
static void set_eo(cube_t *, long long);

static int h_cp5(cube_t);
static struct table *tetrad_twist_table;
static int get_tetrad_twist(cube_t);
static void set_tetrad_twist(cube_t *, int);

#endif
