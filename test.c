#include "common.h"

#include "data.h"
#include "cube.h"
#include "moves.h"
#include "util.h"
#include "table.h"

#include "data.c"
#include "cube.c"
#include "moves.c"
#include "util.c"
#include "table.c"

#define TEST(x) for (assert(!name), assert(x), name=(x), printf("running test '%s'\n", name); name; name=0)
#define CHECK(x, y) ((x)!=(y) ? fail((x), (y), #x, #y) : (void)0)

static char *name;

static void fail(int x, int y, char *xs, char *ys)
{
    fprintf(stderr, "test \"%s\" failed (%s:%d, %s:%d)\n", name, xs, x, ys, y);
    exit(1);
}

// NOTE this does not check weather the corner and edge parities are the same
static int cube_valid(cube_t x)
{
    int valid_permutation(char *x, int n)
    {
        int seen[n];
        memset(seen, 0x00, sizeof(int)*n);
        for (int i=0, y; i<n; ++i)
            if ((y=x[i]&0x0f)<0 || y>=n)
                return 0;
            else
                seen[y]++;
        for (int i=0; i<n; ++i)
            if (seen[i]!=1)
                return 0;
        return 1;
    }

    int valid_orientation(char *x, int n, int order)
    {
        int result = 0;
        for (int i=0, y; i<n; ++i)
            if ((y=x[i]>>4)<0 || y>=order)
                return 0;
            else
                result += y;
        return 0==result%order;
    }

    return (valid_permutation(x.corners, NUM_CORNERS)    &&
            valid_permutation(x.edges,   NUM_EDGES)      &&
            valid_orientation(x.corners, NUM_CORNERS, 3) &&
            valid_orientation(x.edges,   NUM_EDGES, 2));
}

int main(void)
{
    TEST("transform")
    {
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            for (int s=0; s<48; ++s)
            {
                int transformed_moves[LENGTH(moves)];
                for (int i=0; i<length; ++i)
                    transformed_moves[i] = sym_moves[s][moves[i]];
                CHECK(cube_eq(apply_sym(x, s), apply_moves(new_cube(), transformed_moves, length)), 1);
            }
        }
    }

    TEST("invert co")
    {
        for (int i=0; i<10; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            cube_t y = invert_co(x);
            for (int i=0; i<NUM_CORNERS; ++i)
                y.corners[i] = (y.corners[i]&0xf0)+i;
            cube_t z = compose(x, y);
            CHECK(get_co(z), 0);
        }
    }

    TEST("combination")
    {
        int n=12, k=4;
        for (int i=0; i<choose[n][k]; ++i)
        {
            char x[n];
            set_combination(x, n, k, i);
            CHECK(i, get_combination(x, n, k));
        }
    }

    TEST("partial permutation")
    {
        int n=12, k=4;
        for (int i=0; i<pick[n][k]; ++i)
        {
            char x[n];
            set_partial_permutation(x, n, k, i);
            CHECK(i, get_partial_permutation(x, n, k));
        }
    }

    TEST("permutation")
    {
        int n=8;
        for (int i=0; i<fact[n]; ++i)
        {
            char x[n];
            set_permutation(x, n, i);
            CHECK(i, get_permutation(x, n));
        }
    }

    printf("all tests passed\n");
    return 0;
}
