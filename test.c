#include "common.h"

#include "data.h"
#include "cube.h"
#include "moves.h"
#include "table.h"

#include "data.c"
#include "cube.c"
#include "moves.c"
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

    TEST("eo")
    {
        long long n=pow2[11];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_eo(set_eo(i)));
    }

    TEST("co")
    {
        long long n=pow3[7];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_co(set_co(i)));
    }


    TEST("csep")
    {
        long long n=choose[8][4];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_csep(set_csep(i)));
    }

    TEST("esep")
    {
        long long n=choose[12][4]*choose[8][4];
        for (long long i=0; i<n; ++i)
            CHECK(i, get_esep(set_esep(i)));
    }

    TEST("invert co")
    {
        for (int i=0; i<10; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            cube_t y = set_co(get_co(invert_co(x)));
            cube_t z = compose(x, y);
            CHECK(get_co(z), 0);
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

    printf("all tests passed\n");
    return 0;
}
