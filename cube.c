#include "common.h"

// todo maybe move these to another file
static int pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};
static int fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800};

static int choose(int n, int r)
{
    return n>=r ?  fact[n]/fact[r]/fact[n-r] : 0;
}

static int combination_index(char *x, int n)
{
    int result=0;
    for (int i=0; i<n; ++i)
        result += choose(x[i], i+1);
    return result;
}

static int permutation_index(char *x, int n)
{
    unsigned b = 0;
    int result = 0;
    for (int i=0; i<n; ++i)
    {
        b |= 1 << ((n-1)-x[i]);
        unsigned s = b >> (n-x[i]);
        int c = x[i]-__builtin_popcount(s);
        result += c*fact[(n-1)-i];
    }
    return result;
}

static void inv_permutation_index(char *x, int n, int r)
{
    unsigned b = (1<<n)-1;
    for (int i=0; i<n; ++i)
    {
        int c = r/fact[n-1-i];
        x[i] = c;
        unsigned s;
        while (s=b>>(n-1-x[i]), __builtin_popcount(s)<=c) ++x[i];
        b ^= 1<<(n-1-x[i]);
        r %= fact[n-1-i];
    }
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

static cube move_table[] = {
    //      URF ULB DRB DLF URB ULF DRF DLB  UR  UL  DR  DL  UF  UB  DF  DB  RF  RB  LF  LB
    [U]  = {  5,  4,  2,  3,  0,  1,  6,  7,  4,  5,  2,  3,  1,  0,  6,  7,  8,  9, 10, 11},
    [R]  = { 36,  1, 38,  3, 18,  5, 16,  7,  9,  1,  8,  3,  4,  5,  6,  7,  0,  2, 10, 11},
    [F]  = { 22,  1,  2, 21,  4, 32, 35,  7,  0,  1,  2,  3, 24,  5, 26,  7, 22,  9, 20, 11},
    [D]  = {  0,  1,  7,  6,  4,  5,  2,  3,  0,  1,  7,  6,  4,  5,  2,  3,  8,  9, 10, 11},
    [L]  = {  0, 37,  2, 39,  4, 19,  6, 17,  0, 10,  2, 11,  4,  5,  6,  7,  8,  9,  3,  1},
    [B]  = {  0, 23, 20,  3, 33,  5,  6, 34,  0,  1,  2,  3,  4, 27,  6, 25,  8, 21, 10, 23},
    [U2] = {  1,  0,  2,  3,  5,  4,  6,  7,  1,  0,  2,  3,  5,  4,  6,  7,  8,  9, 10, 11},
    [R2] = {  2,  1,  0,  3,  6,  5,  4,  7,  2,  1,  0,  3,  4,  5,  6,  7,  9,  8, 10, 11},
    [F2] = {  3,  1,  2,  0,  4,  6,  5,  7,  0,  1,  2,  3,  6,  5,  4,  7, 10,  9,  8, 11},
    [D2] = {  0,  1,  3,  2,  4,  5,  7,  6,  0,  1,  3,  2,  4,  5,  7,  6,  8,  9, 10, 11},
    [L2] = {  0,  3,  2,  1,  4,  7,  6,  5,  0,  3,  2,  1,  4,  5,  6,  7,  8,  9, 11, 10},
    [B2] = {  0,  2,  1,  3,  7,  5,  6,  4,  0,  1,  2,  3,  4,  7,  6,  5,  8, 11, 10,  9},
    [U3] = {  4,  5,  2,  3,  1,  0,  6,  7,  5,  4,  2,  3,  0,  1,  6,  7,  8,  9, 10, 11},
    [R3] = { 38,  1, 36,  3, 16,  5, 18,  7,  8,  1,  9,  3,  4,  5,  6,  7,  2,  0, 10, 11},
    [F3] = { 21,  1,  2, 22,  4, 35, 32,  7,  0,  1,  2,  3, 26,  5, 24,  7, 20,  9, 22, 11},
    [D3] = {  0,  1,  6,  7,  4,  5,  3,  2,  0,  1,  6,  7,  4,  5,  3,  2,  8,  9, 10, 11},
    [L3] = {  0, 39,  2, 37,  4, 17,  6, 19,  0, 11,  2, 10,  4,  5,  6,  7,  8,  9,  1,  3},
    [B3] = {  0, 20, 23,  3, 34,  5,  6, 33,  0,  1,  2,  3,  4, 25,  6, 27,  8, 23, 10, 21},
};

// use struct/constant instead of function that returns one
cube new_cube(void)
{
    cube x;
    for (int i=0; i<NUM_CORNERS; ++i) x.corners[i]=i;
    for (int i=0; i<NUM_EDGES; ++i) x.edges[i]=i;
    return x;
}

void print_cube(cube x)
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
        result.corners[i] = y.corners[x.corners[i] & PERM_MASK];
        result.corners[i] += x.corners[i] & ORIENT_MASK;
        result.corners[i] %= 3*0x10;
    }

    for (int i=0; i<NUM_EDGES; ++i)
    {
        result.edges[i] = y.edges[x.edges[i] & PERM_MASK];
        result.edges[i] += x.edges[i] & ORIENT_MASK;
        result.edges[i] %= 2*0x10;
    }

    return result;
}

static cube separate_corners(cube x)
{
    cube result = x;
    int j=0, k=0;
    for (int i=0; i<NUM_CORNERS; ++i)
        if (x.corners[i]<4)
            result.tetrads[0][j++] = x.corners[i];
        else
            result.tetrads[1][k++] = x.corners[i];
    assert(j==4 && k==4);
    return result;
}

cube apply_move(cube x, int move)
{
    return compose(x, move_table[move]);
}

cube apply_moves(cube x, int *moves, int length)
{
    cube result=x;
    for (int i=0; i<length; ++i) result=apply_move(result, moves[i]);
    return result;
}

static void get_orbit_ordered(char *cubies, int j, int length, char orbit[4])
{
    for (int i=0; i<4; ++i) orbit[i]=cubies[j*4+i]&PERM_MASK;
}

static void get_tetrad_ordered(cube x, int j, char tetrad[4])
{
    get_orbit_ordered(x.corners, j, NUM_CORNERS, tetrad);
}

static void get_slice_ordered(cube x, int j, char slice[4])
{
    get_orbit_ordered(x.edges, j, NUM_EDGES, slice);
}

static void get_orbit(char *cubies, int j, int length, char orbit[4])
{
    get_orbit_ordered(cubies, j, length, orbit);
    // https://bertdobbelaere.github.io/sorting_networks.html#N4L5D3
    #define SORT(i, j) do if (orbit[i]>orbit[j]) SWAP(orbit[i], orbit[j]); while (0)
    SORT(0,2); SORT(1,3);
    SORT(0,1); SORT(2,3);
    SORT(1,2);
    #undef SORT
}

static void get_tetrad(cube x, int j, char tetrad[4])
{
    get_orbit(x.corners, j, NUM_CORNERS, tetrad);
}

static void get_slice(cube x, int j, char slice[4])
{
    get_orbit(x.edges, j, NUM_EDGES, slice);
}

///////////////////////////////////////////////////////////////////////////////

static int index_co(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_CORNERS-1; ++i)
    {
        int cp = x.corners[i]&PERM_MASK;
        int co = x.corners[cp]>>4;
        result += co*pow3[i];
    }
    return result;
}

static int index_eo(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
    {
        int ep = x.edges[i]&PERM_MASK;
        int eo = x.edges[ep]>>4;
        result += eo<<i;
    }
    return result;
}

static int index_orbit(char *cubies, int j, int length)
{
    char     orbit[4];
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
            result ^= (x.corners[i]&PERM_MASK)<(x.corners[j]&PERM_MASK);
    return result;
}

// note this does not actually index the twist [0,6) it instead says weather it's solved or not
static int index_tetrad_twist(cube x)
{
    return table_get(tetrad_twist_table, permutation_index(x.corners, NUM_CORNERS));
}

///////////////////////////////////////////////////////////////////////////////

static int goal_tw_g0(cube x)
{
    int result = index_eo(x);
    return result == 0;
}

static int goal_tw_g1(cube x)
{
    int result = index_co(x) + pow3[NUM_CORNERS-1]*index_slice(x, SLICE_UD);
    return result == 1080378;
}

static int goal_tw_g2(cube x)
{
    int result = 0;
    result += index_slice(x, SLICE_FB);
    result += index_tetrad(x) * choose(8, 4);
    result += index_tetrad_twist(x) * choose(8, 4) * choose(8, 4);
    return result == 69;
}

static int goal_tw_g3(cube x)
{
    static int powfact4[] = {1, 24, 576, 13824, 331776};

    char orbits[5][4];
    get_tetrad_ordered(x, 0, orbits[0]);
    get_tetrad_ordered(x, 1, orbits[1]);
    get_slice_ordered(x, 0, orbits[2]);
    get_slice_ordered(x, 1, orbits[3]);
    get_slice_ordered(x, 2, orbits[4]);

    // since cubies are in their tetrad/slice, number them from 0-3 within their tetrad
    for (int i=0; i<5; ++i) for (int j=0; j<4; ++j) orbits[i][j] %= 4;

    int result = 0;
    result += permutation_index(orbits[0], 4) * powfact4[0];
    result += permutation_index(orbits[2], 4) * powfact4[1];
    result += permutation_index(orbits[3], 4) * powfact4[2];
    result += orbits[1][0]                    * powfact4[3];
    // this isn't the most compact way of indexing the last slice
    // todo use pick perm
    result += orbits[4][0]                    * powfact4[3] * 4;
    result += orbits[4][1]                    * powfact4[3] * 16;
    return result == 221184;
}

struct
{
    int (*goal)(cube);
    int quater_turns[6];
} stages[] =
{
    {goal_tw_g0, {1, 1, 1, 1, 1, 1}},
    {goal_tw_g1, {1, 1, 0, 1, 1, 0}},
    {goal_tw_g2, {1, 0, 0, 1, 0, 0}},
    {goal_tw_g3, {0, 0, 0, 0, 0, 0}},
};

static int dls(cube x, int *path, int max_depth, int (*goal)(cube), int quater_turns[6])
{
    typedef struct
    {
        cube cube;
        int move;
        int depth;
    } node;
    node stack[2048] = {{.cube=x}}; // todo look at different stack sizes
    node *top = 1+stack;
    while (top>stack)
    {
        node cur = *--top;
        if (cur.depth)
            path[cur.depth-1] = cur.move;
        if (goal(cur.cube))
            return 0;
        if (cur.depth == max_depth)
            continue;

        for (int i=0; i<LENGTH(move_set); ++i)
        {
            int face=move_set[i]%6;
            int n=1+move_set[i]/U2;
            if (n!=2 && !quater_turns[face])
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

static int iddfs(cube x, int *path, int (*goal)(cube), int quater_turns[6])
{
    int depth = 0;
    while (dls(x, path, depth, goal, quater_turns)) ++depth;
    return depth;
}

void solve(cube x, int *path, int *length, int (*goal)(cube), int quater_turns[6])
{
    *length = iddfs(x, path, goal, quater_turns);
}

void thistlethwaite(cube x, int *path, int *length)
{
    *length = 0;
    for (int i=0; i<LENGTH(stages); ++i)
    {
        int depth = iddfs(x, path+*length, stages[i].goal, stages[i].quater_turns);
        x = apply_moves(x, path+*length, depth);
        #if 1
        printf("stage%d: ", i);
        print_moves(path+*length, depth);
        printf(" // %d move%s\n", depth, depth==1?"":"s");
        #endif
        *length += depth;
    }
}

///////////////////////////////////////////////////////////////////////////////

static int goal_cp5(cube x)
{
    for (int i=3; i<NUM_CORNERS; ++i) if (x.corners[i]!=i) return 0;
    return 1;
}

table tetrad_twist_table;
table init_tetrad_twist_table(void)
{
    int n = fact[8];
    table t = table_new(n, 1, "tetrad-twist");
    if (table_read(t)) return t;

    for (int i=0; i<n; ++i)
    {
        #if 1
        cube x = new_cube();
        inv_permutation_index(x.corners, NUM_CORNERS, i);
        x = separate_corners(x);
        // todo function for this
        int moves[64], length;
        solve(x, moves, &length, goal_cp5, stages[3].quater_turns);
        x = apply_moves(x, moves, length);
        table_set(t, i, permutation_index(x.corners, 3)>0);
        #else
        table_set(t, i, permutation_index(
            solve(seperate_corners(inv_index_cp(i)), goal_cp5),
            3
        ));
        #endif
    }

    table_write(t); // todo error handling
    return t;
}

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
