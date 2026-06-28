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
static cube_t cube_canonicalise(cube_t, int *);
static cube_t compose(cube_t, cube_t);
static cube_t apply_sym(cube_t, int);
static cube_t apply_move(cube_t, int);
static cube_t apply_moves(cube_t, int *, int);

static long long get_eo(cube_t);
static void set_eo(cube_t *, long long);
static long long get_co(cube_t);
static void set_co(cube_t *, long long);
static long long get_csep(cube_t);
static void set_csep(cube_t *, long long);
static long long get_esep(cube_t);
static void set_esep(cube_t *, long long);
static long long get_slice(cube_t);
static void set_slice(cube_t *, long long);
static long long get_cp(cube_t);
static void set_cp(cube_t *, long long);
static long long get_ep(cube_t);
static void set_ep(cube_t *, long long);
static long long get_orbit_fast(cube_t);
static void set_orbit_fast(cube_t *, long long);

static inline int check_get_parity(cube_t x)
{
    char a[32];
    _mm256_storeu_si256((__m256i *)a, x);
    for (int i=0; i<LENGTH(a); ++i)
        a[i]&=0x0f,printf("%3d",(int)a[i]);

    int parity(char *a, int n)
    {
        int r=0;
        for (int j=0; j<n; ++j)
            for (int i=0; i<j; ++i)
                r+=a[i]>a[j];
        return r%2;
    }

    return parity(a,12)<<1|parity(a+16,8);
}

static cube_t inverse(cube_t);

#endif
