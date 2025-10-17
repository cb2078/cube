#include "coord.h"
#include "table.h"
#include "util.h"

static int get_tw_g0(cube x)
{
    int result = 0;
    result += get_eo(x);
    return result;
}

static int get_tw_g1(cube x)
{
    int result = 0;
    result += get_co(x);
    result += get_combination(x.slices[SLICE_UD], 12, 4) * pow3[NUM_CORNERS-1];
    return result;
}

static int get_tw_g2(cube x)
{
    int result = 0;
    for (int i=0; i<8; ++i) x.slices[SLICE_RL][i] -= 4;
    result += get_combination(x.slices[SLICE_RL], 8, 4);
    result += get_combination(x.tetrads[TETRAD_URF], 8, 4) * choose(8, 4);
    result += get_tetrad_twist(x) * choose(8, 4) * choose(8, 4);
    return result;
}

static int get_tw_g3(cube x)
{
    int result = 0;
    result += get_permutation(x.tetrads[TETRAD_URF], 4);
    result += get_permutation(x.slices[SLICE_UD], 4) * fact[4];
    for (int i=0; i<4; ++i) x.slices[SLICE_RL][i] -= 4;
    result += get_permutation(x.slices[SLICE_RL], 4) * fact[4] * fact[4];
    for (int i=0; i<4; ++i) x.slices[SLICE_FB][i] -= 8;
    result += get_partial_permutation(x.slices[SLICE_FB], 4, 2) * fact[4] * fact[4] * fact[4];
    for (int i=0; i<4; ++i) x.tetrads[TETRAD_URB][i] -= 4;
    result += get_partial_permutation(x.tetrads[TETRAD_URB], 4, 1) * fact[4] * fact[4] * fact[4] * pick(4, 2);
    return result;
}

static cube set_tw_g0(int result)
{
    cube x = new_cube();
    int i;
    i = result%pow2[NUM_EDGES-1];
    set_eo(&x, i);
    return x;
}

static cube set_tw_g1(int result)
{
    cube x = new_cube();
    int i;
    i = result%pow3[NUM_CORNERS-1];
    set_co(&x, i);
    result /= pow3[NUM_CORNERS-1];
    i = result%choose(12, 4);
    set_combination(x.slices[SLICE_UD], 12, 4, i);
    return x;
}

static cube set_tw_g2(int result)
{
    cube x = new_cube();
    int i;
    i = result%choose(8, 4);
    set_combination(x.slices[SLICE_RL], 8, 4, i);
    for (int i=0; i<8; ++i) x.slices[SLICE_RL][i] += 4;
    result /= choose(8, 4);
    i = result%choose(8, 4);
    set_combination(x.tetrads[TETRAD_URF], 8, 4, i);
    result /= choose(8, 4);
    i = result%6;
    set_tetrad_twist(&x, i);
    return x;
}

static cube set_tw_g3(int result)
{
    cube x = new_cube();
    int i;
    i = result%fact[4];
    set_permutation(x.tetrads[TETRAD_URF], 4, i);
    result /= fact[4];
    i = result%fact[4];
    set_permutation(x.slices[SLICE_UD], 4, i);
    result /= fact[4];
    i = result%fact[4];
    set_permutation(x.slices[SLICE_RL], 4, i);
    for (int i=0; i<4; ++i) x.slices[SLICE_RL][i] += 4;
    result /= fact[4];
    i = result%pick(4, 2);
    set_partial_permutation(x.slices[SLICE_FB], 4, 2, i);
    for (int i=0; i<4; ++i) x.slices[SLICE_FB][i] += 8;
    result /= pick(4, 2);
    i = result%pick(4, 1);
    set_partial_permutation(x.tetrads[TETRAD_URB], 4, 1, i);
    for (int i=0; i<4; ++i) x.tetrads[TETRAD_URB][i] += 4;
    return x;
}

static int h_tw_g0(cube x)
{
    return table_get(tw_coords[0].table, tw_coords[0].get(x));
}

static int h_tw_g1(cube x)
{
    return table_get(tw_coords[1].table, tw_coords[1].get(x));
}

static int h_tw_g2(cube x)
{
    return table_get(tw_coords[2].table, tw_coords[2].get(x));
}

static int h_tw_g3(cube x)
{
    return table_get(tw_coords[3].table, tw_coords[3].get(x));
}

coord tw_coords[] = 
{
    {.name="tw_g0", .get=get_tw_g0, .set=set_tw_g0, .h=h_tw_g0, .quater_turns={1, 1, 1, 1, 1, 1}, .order=2048},
    {.name="tw_g1", .get=get_tw_g1, .set=set_tw_g1, .h=h_tw_g1, .quater_turns={1, 1, 0, 1, 1, 0}, .order=1082565},
    {.name="tw_g2", .get=get_tw_g2, .set=set_tw_g2, .h=h_tw_g2, .quater_turns={1, 0, 0, 1, 0, 0}, .order=29400},
    {.name="tw_g3", .get=get_tw_g3, .set=set_tw_g3, .h=h_tw_g3, .quater_turns={0, 0, 0, 0, 0, 0}, .order=663552},
};
