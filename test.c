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

#include "test.inc"

static char *name;

static void fail(int x, int y, char *xs, char *ys)
{
    fprintf(stderr, "test \"%s\" failed (%s:%d, %s:%d)\n", name, xs, x, ys, y);
    exit(1);
}

int main(void)
{
    TEST("moves")
    {
        ASSERT(LENGTH(scrambles) == LENGTH(solutions));
        for (int i=0; i<LENGTH(scrambles); ++i)
        {
            cube_t x = apply_moves(new_cube(), scrambles[i], 9);
            cube_t y = apply_moves(new_cube(), solutions[i], 9);
            cube_t z = compose(x, y);
            CHECK(cube_eq(z, new_cube()), 1);
        }
    }

    TEST("inverse")
    {
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            cube_t y = inverse(x);
            cube_t z = compose(x, y);
            CHECK(cube_eq(z, new_cube()), 1);
        }
    }

    TEST("flip")
    {
        long long n=pow2[11];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_flip(set_flip(i)));
    }

    TEST("twist")
    {
        long long n=pow3[7];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_twist(set_twist(i)));
    }


    TEST("corner sep")
    {
        long long n=choose[8][4];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_corner_sep(set_corner_sep(i)));
    }

    TEST("edge sep")
    {
        long long n=choose[12][4]*choose[8][4];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_edge_sep(set_edge_sep(i)));
    }

    TEST("invert twist")
    {
        for (int i=0; i<10; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            cube_t y = set_twist(get_twist(invert_twist(x)));
            cube_t z = compose(x, y);
            CHECK(get_twist(z), 0);
        }
    }

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
