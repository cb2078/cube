#include <assert.h>

#include "common.h"

#define PERM_MASK   0x0f
#define ORIENT_MASK 0xf0

union cube
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
    struct
    {
        char cubies[NUM_CORNERS+NUM_EDGES];
    };
};
typedef union cube cube;
// static_assert(sizeof(cube)==20, "");

static int move_set[18] = {
    U,  R,  F,  D,  L,  B,
    U2, R2, F2, D2, L2, B2,
    U3, R3, F3, D3, L3, B3
};

static cube move_table[] = {
    #define CUBE(...) {{__VA_ARGS__}}
    //          URF ULB DRB DLF URB ULF DRF DLB  UR  UL  DR  DL  UF  UB  DF  DB  RF  RB  LF  LB
    [U]  = CUBE(  4,  5,  2,  3,  1,  0,  6,  7,  5,  4,  2,  3,  0,  1,  6,  7,  8,  9, 10, 11),
    [R]  = CUBE( 38,  1, 36,  3, 16,  5, 18,  7,  8,  1,  9,  3,  4,  5,  6,  7,  2,  0, 10, 11),
    [F]  = CUBE( 21,  1,  2, 22,  4, 35, 32,  7,  0,  1,  2,  3, 26,  5, 24,  7, 20,  9, 22, 11),
    [D]  = CUBE(  0,  1,  6,  7,  4,  5,  3,  2,  0,  1,  6,  7,  4,  5,  3,  2,  8,  9, 10, 11),
    [L]  = CUBE(  0, 39,  2, 37,  4, 17,  6, 19,  0, 11,  2, 10,  4,  5,  6,  7,  8,  9,  1,  3),
    [B]  = CUBE(  0, 20, 23,  3, 34,  5,  6, 33,  0,  1,  2,  3,  4, 25,  6, 27,  8, 23, 10, 21),
    [U2] = CUBE(  1,  0,  2,  3,  5,  4,  6,  7,  1,  0,  2,  3,  5,  4,  6,  7,  8,  9, 10, 11),
    [R2] = CUBE(  2,  1,  0,  3,  6,  5,  4,  7,  2,  1,  0,  3,  4,  5,  6,  7,  9,  8, 10, 11),
    [F2] = CUBE(  3,  1,  2,  0,  4,  6,  5,  7,  0,  1,  2,  3,  6,  5,  4,  7, 10,  9,  8, 11),
    [D2] = CUBE(  0,  1,  3,  2,  4,  5,  7,  6,  0,  1,  3,  2,  4,  5,  7,  6,  8,  9, 10, 11),
    [L2] = CUBE(  0,  3,  2,  1,  4,  7,  6,  5,  0,  3,  2,  1,  4,  5,  6,  7,  8,  9, 11, 10),
    [B2] = CUBE(  0,  2,  1,  3,  7,  5,  6,  4,  0,  1,  2,  3,  4,  7,  6,  5,  8, 11, 10,  9),
    [U3] = CUBE(  5,  4,  2,  3,  0,  1,  6,  7,  4,  5,  2,  3,  1,  0,  6,  7,  8,  9, 10, 11),
    [R3] = CUBE( 36,  1, 38,  3, 18,  5, 16,  7,  9,  1,  8,  3,  4,  5,  6,  7,  0,  2, 10, 11),
    [F3] = CUBE( 22,  1,  2, 21,  4, 32, 35,  7,  0,  1,  2,  3, 24,  5, 26,  7, 22,  9, 20, 11),
    [D3] = CUBE(  0,  1,  7,  6,  4,  5,  2,  3,  0,  1,  7,  6,  4,  5,  2,  3,  8,  9, 10, 11),
    [L3] = CUBE(  0, 37,  2, 39,  4, 19,  6, 17,  0, 10,  2, 11,  4,  5,  6,  7,  8,  9,  3,  1),
    [B3] = CUBE(  0, 23, 20,  3, 33,  5,  6, 34,  0,  1,  2,  3,  4, 27,  6, 25,  8, 21, 10, 23),
    #undef CUBE
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

///////////////////////////////////////////////////////////////////////////////

static int index_eo(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
        result += (x.edges[i]>>4!=0)<<i;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

// for now, solve EO and return number of moves
static int dls(cube x, int *path, int max_depth)
{
    typedef struct
    {
        cube cube;
        int move;
        int depth;
    } node;
    node stack[2048] = {{x, 0xff, 0}};
    node *top = 1+stack;
    while (top>stack)
    {
        node cur = *--top;
        if (cur.depth)
            path[cur.depth-1] = cur.move;
        if (index_eo(cur.cube) == 0)
            return 0;

        for (int i=0; i<LENGTH(move_set); ++i)
        {
            node next = {apply_move(cur.cube, move_set[i]), move_set[i], cur.depth+1};
            if (next.depth > max_depth) continue;
            assert(top-stack < 2048);
            *top++ = next;
        }
    }
    return 1;
}

static int iddfs(cube x, int *path)
{
    int depth=0;
    while (dls(x, path, depth)) ++depth;
    return depth;
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
