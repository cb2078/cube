#include "common.h"

#include "coord.h"
#include "cube.h"
#include "data.h"
#include "map.h"
#include "moves.h"
#include "prune.h"
#include "solver.h"
#include "table.h"

#include "coord.c"
#include "cube.c"
#include "data.c"
#include "map.c"
#include "moves.c"
#include "prune.c"
#include "solver.c"
#include "table.c"

#define CHECK(x, y) ((x)!=(y) ? fail((x), (y), #x, #y) : (void)0)
#define TEST(x) for (assert(!name), assert(x), name=(x), printf("running test '%s'\n", name); name; name=0)

#define TEST_COORD(NAME, CAPS)\
    TEST(#NAME)\
        for (long long i=0; i<CAPS##_MAX; ++i)\
        {\
            cube_t x = new_cube();\
            set_##NAME(&x, i);\
            CHECK(i, get_##NAME(x));\
        }\

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

    TEST("syms")
    {
        int moves[256];
        int transformed_moves[LENGTH(moves)];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                for (int i=0; i<length; ++i)
                    transformed_moves[i] = sym_moves[s%48][moves[i]];
                if (s>47)
                    inverse_moves(transformed_moves, length);
                CHECK(cube_eq(apply_sym(x, s), apply_moves(new_cube(), transformed_moves, length)), 1);
            }
        }
    }

    TEST_COORD(eo, EO);
    TEST_COORD(co, CO);
    TEST_COORD(csep, CSEP);
    TEST_COORD(esep, ESEP);

    printf("all tests passed\n");
    return 0;
}
