#include <assert.h>

#include "common.h"

// todo maybe move these to another file
static int pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};
static int fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800};

static int choose(int n, int r)
{
    return n>=r ?  fact[n]/fact[r]/fact[n-r] : 0;
}

static int combination_index(int *x, int n)
{
    int result=0;
    for (int i=0; i<n; ++i)
        result += choose(x[i], i+1);
    return result;
}

static int permutation_index(unsigned *x, int n)
{
    unsigned b = 0;
    unsigned result = 0;
    for (int i=0; i<n; ++i)
    {
        b |= 1 << ((n-1)-x[i]);
        unsigned s = b >> (n-x[i]);
        unsigned c = x[i]-__builtin_popcount(s);
        result += c*fact[(n-1)-i];
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

#define PERM_MASK   0x0f
#define ORIENT_MASK 0xf0

enum slice
{
    SLICE_RL,
    SLICE_FB,
    SLICE_UD,
};

typedef union
{
    struct
    {
        char urf, ulb, drb, dlf;
        char urb, ulf, drl, dlb;
        char ur, ul, dr, dl;
        char uf, ub, df, db;
        char rf, rb, lf, lb;
    };
    struct
    {
        char corners[NUM_CORNERS];
        char edges[NUM_EDGES];
    };
    char cubies[NUM_CORNERS+NUM_EDGES];
} cube;
// static_assert(sizeof(cube)==20, "");

static int move_set[18] = {
    U,  R,  F,  D,  L,  B,
    U2, R2, F2, D2, L2, B2,
    U3, R3, F3, D3, L3, B3
};

static cube move_table[] = {
    //      URF ULB DRB DLF URB ULF DRF DLB  UR  UL  DR  DL  UF  UB  DF  DB  RF  RB  LF  LB
    [U]  = {  4,  5,  2,  3,  1,  0,  6,  7,  5,  4,  2,  3,  0,  1,  6,  7,  8,  9, 10, 11},
    [R]  = { 38,  1, 36,  3, 16,  5, 18,  7,  8,  1,  9,  3,  4,  5,  6,  7,  2,  0, 10, 11},
    [F]  = { 21,  1,  2, 22,  4, 35, 32,  7,  0,  1,  2,  3, 26,  5, 24,  7, 20,  9, 22, 11},
    [D]  = {  0,  1,  6,  7,  4,  5,  3,  2,  0,  1,  6,  7,  4,  5,  3,  2,  8,  9, 10, 11},
    [L]  = {  0, 39,  2, 37,  4, 17,  6, 19,  0, 11,  2, 10,  4,  5,  6,  7,  8,  9,  1,  3},
    [B]  = {  0, 20, 23,  3, 34,  5,  6, 33,  0,  1,  2,  3,  4, 25,  6, 27,  8, 23, 10, 21},
    [U2] = {  1,  0,  2,  3,  5,  4,  6,  7,  1,  0,  2,  3,  5,  4,  6,  7,  8,  9, 10, 11},
    [R2] = {  2,  1,  0,  3,  6,  5,  4,  7,  2,  1,  0,  3,  4,  5,  6,  7,  9,  8, 10, 11},
    [F2] = {  3,  1,  2,  0,  4,  6,  5,  7,  0,  1,  2,  3,  6,  5,  4,  7, 10,  9,  8, 11},
    [D2] = {  0,  1,  3,  2,  4,  5,  7,  6,  0,  1,  3,  2,  4,  5,  7,  6,  8,  9, 10, 11},
    [L2] = {  0,  3,  2,  1,  4,  7,  6,  5,  0,  3,  2,  1,  4,  5,  6,  7,  8,  9, 11, 10},
    [B2] = {  0,  2,  1,  3,  7,  5,  6,  4,  0,  1,  2,  3,  4,  7,  6,  5,  8, 11, 10,  9},
    [U3] = {  5,  4,  2,  3,  0,  1,  6,  7,  4,  5,  2,  3,  1,  0,  6,  7,  8,  9, 10, 11},
    [R3] = { 36,  1, 38,  3, 18,  5, 16,  7,  9,  1,  8,  3,  4,  5,  6,  7,  0,  2, 10, 11},
    [F3] = { 22,  1,  2, 21,  4, 32, 35,  7,  0,  1,  2,  3, 24,  5, 26,  7, 22,  9, 20, 11},
    [D3] = {  0,  1,  7,  6,  4,  5,  2,  3,  0,  1,  7,  6,  4,  5,  2,  3,  8,  9, 10, 11},
    [L3] = {  0, 37,  2, 39,  4, 19,  6, 17,  0, 10,  2, 11,  4,  5,  6,  7,  8,  9,  3,  1},
    [B3] = {  0, 23, 20,  3, 33,  5,  6, 34,  0,  1,  2,  3,  4, 27,  6, 25,  8, 21, 10, 23},
};

// use struct/constant instead of function that returns one
static cube new_cube(void)
{
    cube x;
    for (int i=0; i<NUM_CORNERS; ++i) x.corners[i]=i;
    for (int i=0; i<NUM_EDGES; ++i) x.edges[i]=i;
    return x;
}

static void print_cube(cube x)
{
    #define PRINT(x, ...) \
        printf("%2s ", x); \
        for (int i=0; i<NUM_CORNERS+NUM_EDGES; ++i) \
            printf(__VA_ARGS__); \
        printf("\n");
    PRINT("",   "%s%3s", i?" ":"", cubie_str[i]);
    PRINT("p:", "%s%3d", i?",":"", x.cubies[i]&PERM_MASK);
    PRINT("o:", "%s%3d", i?",":"", x.cubies[i]>>4);
    #undef PRINT
}

static int cube_eq(cube x, cube y)
{
    return 0==memcmp(&x, &y, sizeof(cube));
}

static cube compose(cube x, cube y)
{
    cube result;

    for (int i=0; i<NUM_CORNERS; ++i)
    {
        result.corners[i] = x.corners[y.corners[i] & PERM_MASK];
        result.corners[i] += y.corners[i] & ORIENT_MASK;
        result.corners[i] %= 3*0x10;
    }

    for (int i=0; i<NUM_EDGES; ++i)
    {
        result.edges[i] = x.edges[y.edges[i] & PERM_MASK];
        result.edges[i] += y.edges[i] & ORIENT_MASK;
        result.edges[i] %= 2*0x10;
    }

    return result;
}

static cube apply_move(cube x, int move)
{
    return compose(x, move_table[move]);
}

static cube apply_moves(cube x, int *moves, int length)
{
    cube result=x;
    for (int i=0; i<length; ++i) result=apply_move(result, moves[i]);
    return result;
}

static void get_orbit(char *cubies, int j, int length, int orbit[4])
{
    for (int i=0, n=0; i<length; ++i)
        if ((cubies[i]&PERM_MASK)/4 == j)
            orbit[n++] = i;
}

static void get_tetrad(cube x, int j, int tetrad[4])
{
    get_orbit(x.corners, j, NUM_CORNERS, tetrad);
}

static void get_slice(cube x, int j, int slice[4])
{
    get_orbit(x.edges, j, NUM_EDGES, slice);
}

static void get_orbit_ordered(char *cubies, int j, int length, int orbit[4])
{
    for (int i=0; i<length; ++i)
        if ((cubies[i]&PERM_MASK)/4 == j)
            orbit[(cubies[i]&PERM_MASK)%4] = i;
}

static void get_tetrad_ordered(cube x, int j, int tetrad[4])
{
    get_orbit_ordered(x.corners, j, NUM_CORNERS, tetrad);
}

static void get_slice_ordered(cube x, int j, int slice[4])
{
    get_orbit_ordered(x.edges, j, NUM_EDGES, slice);
}

///////////////////////////////////////////////////////////////////////////////

static int move_face(int x)
{
    return x%6;
}

static int move_axis(int x)
{
    return x%3;
}

static int move_amount(int x)
{
    return x/U2+1;
}

static int inverse_move(int x)
{
    return move_face(x) + U2*(3-move_amount(x));
}

static int prune_move(int x, int y)
{
    return move_axis(x)==move_axis(y) && move_face(x)>=move_face(y);
}

static void print_moves(int *moves, int length)
{
    for (int i=0; i<length; ++i) printf("%s%s", i?" ":"", move_str[moves[i]]);
}

static void make_scramble(int *moves, int length)
{
    for (int i=0; i<length; i+=i==0||!prune_move(moves[i-1], moves[i]))
        moves[i]=move_set[rand()%LENGTH(move_set)];
}

static int apply_cancellations(int *moves, int *length)
{
    int cancelled=0;
    for (int i=*length-2; i>=0; --i)
    {
        if (!prune_move(moves[i], moves[i+1]))
            continue;
        if (move_face(moves[i])==move_face(moves[i+1]))
        {
            int amount=(move_amount(moves[i])+move_amount(moves[i+1]))%4;
            if (amount)
            {
                moves[i]=move_face(moves[i])+U2*(amount-1);
                memcpy(&moves[i+1], &moves[i+2], *length-(i+2));
                *length-=1;
            }
            else
            {
                memcpy(&moves[i], &moves[i+2], *length-(i+2));
            }
            cancelled=1;
        }
        else
        {
            SWAP(moves[i], moves[i+1]);
        }
    }
    if (cancelled) apply_cancellations(moves, length);
    return cancelled;
}

static void inverse_moves(int *moves, int length)
{
    for (int i=0; i<length; ++i) moves[i]=inverse_move(moves[i]);
    for (int i=0, j=length-1; i<j; ++i, --j) SWAP(moves[i], moves[j]);
}

///////////////////////////////////////////////////////////////////////////////

static int index_co(cube x)
{
    int result=0;
    for (int i=0; i<NUM_CORNERS-1; ++i)
        result += (x.corners[i]>>4)*pow3[i];
    return result;
}

static int index_eo(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
        result += (x.edges[i]>>4)<<i;
    return result;
}

static int index_orbit(char *cubies, int j, int length)
{
    int orbit[4];
    get_orbit(cubies, j, length, orbit);
    return combination_index(orbit, 4);
}

static int index_tetrad(cube x)
{
    return index_orbit(x.corners, 0, NUM_CORNERS);
}

static int index_slice(cube x, int j)
{
    return index_orbit(x.edges, j, NUM_EDGES);
}

static int index_parity(cube x)
{
    int result=0;
    for (int i=0; i<NUM_CORNERS; ++i)
        for (int j=i+1; j<NUM_CORNERS; ++j)
            result ^= (x.corners[i]&ORIENT_MASK)<(x.corners[j]&ORIENT_MASK);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

static int index_tw_g0(cube x)
{
    return index_eo(x);
}

static int index_tw_g1(cube x)
{
    return index_co(x) + pow3[NUM_CORNERS-1]*index_slice(x, SLICE_UD);
}

static int index_tw_g2(cube x)
{
    int result = 0;
    result += index_slice(x, SLICE_FB);
    result += index_tetrad(x) * choose(8, 4);
    result += index_parity(x) * choose(8, 4) * choose(8, 4);
    return result;
}

static int index_tw_g3(cube x)
{
    unsigned orbits[5][4];
    get_tetrad_ordered(x, 0, (int *)orbits[0]);
    get_tetrad_ordered(x, 1, (int *)orbits[1]);
    get_slice_ordered(x, 0, (int *)orbits[2]);
    get_slice_ordered(x, 1, (int *)orbits[3]);
    get_slice_ordered(x, 2, (int *)orbits[4]);

    // since cubies are in their tetrad/slice, number them from 0-3 within their tetrad
    for (int i=0; i<5; ++i) for (int j=0; j<4; ++j) orbits[i][j] %= 4;

    int result = 0;
    result += permutation_index(orbits[0], 4);
    result += permutation_index(orbits[2], 4) * fact[4];
    result += permutation_index(orbits[3], 4) * fact[4] * fact[4];
    result += orbits[1][0] * fact[4] * fact[4] * fact[4];
    result += orbits[4][0] * fact[4] * fact[4] * fact[4] * 4;
    return result;
}

struct {
    int (*index)(cube);
    int goal;
    int quater_turns[6];
} stages[] = {
    {index_tw_g0, 0,       {1, 1, 1, 1, 1, 1}},
    {index_tw_g1, 1080378, {1, 1, 0, 1, 1, 0}},
    {index_tw_g2, 69,      {1, 0, 0, 1, 0, 0}},
    {index_tw_g3, 0,       {0, 0, 0, 0, 0, 0}},
};

static int dls(cube x, int *path, int max_depth, int stage)
{
    typedef struct
    {
        cube cube;
        int move;
        int depth;
    } node;
    node stack[2048] = {{.cube=x}};
    node *top = 1+stack;
    while (top>stack)
    {
        node cur = *--top;
        if (cur.depth)
            path[cur.depth-1] = cur.move;
        if (stages[stage].index(cur.cube) == stages[stage].goal)
            return 0;
        if (cur.depth == max_depth)
            continue;

        for (int i=0; i<LENGTH(move_set); ++i)
        {
            int face=move_set[i]%6;
            int n=1+move_set[i]/U2;
            if (n!=2 && !stages[stage].quater_turns[face])
                continue;
            if (prune_move(cur.move, move_set[i]))
                continue;
            assert(top-stack < 2048);
            *top++ = (node){
                .cube = apply_move(cur.cube, move_set[i]),
                .move = move_set[i],
                .depth = cur.depth+1,
            };
        }
    }
    return 1;
}

static void iddfs(cube x, int *path, int *length)
{
    *length=0;
    for (int stage=0; stage<LENGTH(stages); ++stage)
    {
        int depth=0;
        while (dls(x, path+*length, depth, stage)) ++depth;
        x=apply_moves(x, path+*length, depth);
        #if 1
        printf("stage%d: ", stage);
        print_moves(path+*length, depth);
        printf(" // %d move%s\n", depth, depth==1?"":"s");
        #endif
        *length+=depth;
    }
}
#define search iddfs

///////////////////////////////////////////////////////////////////////////////

#if 1
static void test(void)
{
    cube x = new_cube();
    for (int i=0; i<LENGTH(move_set); ++i)
    {
        cube y = move_table[move_set[i]];
        assert(cube_eq(compose(x, y), y));
        assert(cube_eq(y, compose(x, y)));
    }

    int jperm[] = {R,U,R3,F3,R,U,R3,U3,R3,F,R2,U3,R3,U3};
    x = apply_moves(x, jperm, LENGTH(jperm));
    x = apply_moves(x, jperm, LENGTH(jperm));
    assert(cube_eq(x, new_cube()));
}
#endif
