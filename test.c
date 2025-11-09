#include "common.h"

#include "coord.h"
#include "cube-table.h"
#include "cube.h"
#include "moves.h"
#include "util.h"
#include "table.h"

#include "coord.c"
#include "cube-table.c"
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
static int cube_valid(cube x)
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
    // check that the sym part of the phase 1 coordinate matches its raw equivalent
    TEST("phase1 sym")
    {
        init_sym(&ko_coords[0]);
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube x = apply_moves(new_cube(), moves, length);
            x = apply_move(new_cube(), R);
            long long eqv_class = get_ko_g0(x)%ko_coords[0].eqv_classes;
            CHECK(ko_coords[0].eqv_class_to_rep[eqv_class],
                  ko_coords[0].coord_to_rep[get_flip_ud_slice(x)]);
            CHECK(ko_coords[0].eqv_class_to_rep[eqv_class],
                  get_flip_ud_slice(apply_sym(x, ko_coords[0].coord_to_rep_sym[get_flip_ud_slice(x)])));
        }
    }

    TEST("phase1 prune")
    {
        init_sym(&ko_coords[0]);
        init_prune_table(&ko_coords[0]);
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube x = apply_moves(new_cube(), moves, length);
            long long a = ko_coords[0].h(x);
            if (a==0) continue;
            int result = 0;
            for (int m=0; m<NUM_FACE_TURNS; ++m)
            {
                long long b = ko_coords[0].h(apply_move(x, m));
                if (b<a) CHECK(a-b, 1), result = 1;
            }
            CHECK(result, 1);
        }
    }

    TEST("basic move transform")
    {
        enum sym
        {
            S_URF3=16, S_U4=4, S_F2=2, S_RL2=1,
        };
        int syms[] = {S_URF3, S_U4, S_F2, S_RL2};
        for (int i=0; i<LENGTH(syms); ++i)
        {
            int moves[] = {R, U, R3, F3, R, U, R3, U3, R3, F, R2, U3, R3, U3};
            int length = LENGTH(moves);
            cube x = apply_sym(apply_moves(new_cube(), moves, length), syms[i]);
            transform_moves(moves, length, syms[i]);
            cube y = apply_moves(new_cube(), moves, length);
#if 1
            CHECK(cube_eq(x, y), 1);
#else
            print_moves(moves, length), putchar('\n');
            gui_show_moves(moves, length);
            gui_wait_for_close();
#endif
        }
    }

    TEST("move transform")
    {
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube x = apply_moves(new_cube(), moves, length);
            for (int j=0; j<LENGTH(sym_table); ++j)
            {
                cube y = apply_sym(x, j);
                int transformed_moves[256];
                memcpy(transformed_moves, moves, sizeof(moves));
                transform_moves(transformed_moves, length, j);
                cube z = apply_moves(new_cube(), transformed_moves, length);
                CHECK(cube_eq(y, z), 1);
            }
        }
    }

    TEST("superflip")
    {
        cube x = new_cube();
        set_eo(&x, 2047);
        for (int j=0; j<16; ++j)
            CHECK(cube_eq(apply_sym(x, j), x), 1);
    }

    TEST("not superflip")
    {
        cube x = new_cube();
        set_eo(&x, 1234);
        int result = 1;
        for (int j=0; j<16; ++j)
            result &= cube_eq(apply_sym(x, j), x);
        CHECK(result, 0);
    }

    TEST("symmetries are valid")
    {
        for (int i=0; i<99; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            cube x = apply_moves(new_cube(), moves, length);
            for (int j=0; j<LENGTH(sym_table); ++j)
                CHECK(cube_valid(apply_sym(x, j)), 1);
        }
    }

    TEST("invert co")
    {
        for (int i=0; i<10; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            cube x = apply_moves(new_cube(), moves, length);
            cube y = invert_co(x);
            for (int i=0; i<NUM_CORNERS; ++i)
                y.corners[i] = (y.corners[i]&0xf0)+i;
            cube z = compose(x, y);
            CHECK(get_co(z), 0);
        }
    }

    TEST("valid cube")
    {
        cube x;
        for (int m=0; m<NUM_FACE_TURNS; ++m)
        {
            x = apply_move(new_cube(), m);
            // print_cube(x);
            CHECK(cube_valid(x), 1);
        }
        for (int i=0; i<10; ++i)
        {
            int moves[256], length=100;
            make_scramble(moves, length);
            x = apply_moves(new_cube(), moves, length);
            // print_cube(x);
            CHECK(cube_valid(x), 1);
        }
    }

    TEST("combination")
    {
        int n=12, k=4;
        for (int i=0; i<choose(n, k); ++i)
        {
            char x[n];
            set_combination(x, n, k, i);
            CHECK(i, get_combination(x, n, k));
        }
    }

    TEST("partial permutation")
    {
        int n=12, k=4;
        for (int i=0; i<pick(n, k); ++i)
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

    TEST("Thistlethwaite coords")
    {
        init_tetrad_twist_table();
        for (int k=0; k<LENGTH(tw_coords); ++k)
        {
            init_sym(&tw_coords[k]);
            for (int i=0; i<tw_coords[k].order; ++i)
            {
                cube x = tw_coords[k].set(i);
                CHECK(cube_valid(x), 1);
                CHECK(tw_coords[k].get(x), i);
            }
        }
    }

    printf("all tests passed\n");
    return 0;
}
