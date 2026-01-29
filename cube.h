#ifndef CUBE_H
#define CUBE_H

#define NUM_CORNERS 8
#define NUM_EDGES 12

#define CUBE(URF, ULB, DRB, DLF, URB, ULF, DRF, DLB, RF, RB, LF, LB, UF, UB, DF, DB, UR, UL, DR, DL)\
    _mm256_set_epi8(15,  14,   13,   12,  11,  10,   9,   8,  DLB, DRF, ULF, URB, DLF, DRB, ULB, URF,\
                    15,  14,   13,   12,  DL,  DR,  UL,  UR,  DB,  DF,  UB,  UF,  LB,  LF,  RB,  RF)

typedef __m256i cube_t;

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
static cube_t apply_sym(cube_t, int);
static cube_t apply_move(cube_t, int);
static cube_t apply_moves(cube_t, int *, int);

static long long get_eo(cube_t);
static cube_t set_eo(long long);
static long long get_co(cube_t);
static cube_t set_co(long long);
static long long get_csep(cube_t);
static cube_t set_csep(long long);
static long long get_esep(cube_t);
static cube_t set_esep(long long);

static cube_t inverse(cube_t);

#endif
