
static long long get_tw_g0(cube x)
{
    long long result=0;

    result += get_eo(x);

    return result;
}

static long long get_tw_g1(cube x)
{
    long long result=0, i=1;

    result += get_co(x);
    i *= pow3[NUM_CORNERS-1];

    result += get_combination(x.slices[SLICE_UD], 12, 4) * i;

    return result;
}

static long long get_tw_g2(cube x)
{
    long long result=0, i=1;

    for (int i=0; i<8; ++i) *(x.slices[SLICE_RL]+i) -= 4;
    result += get_combination(x.slices[SLICE_RL], 8, 4);
    i *= choose(8, 4);

    result += get_combination(x.tetrads[TETRAD_URF], 8, 4) * i;
    i *= choose(8, 4);

    result += get_tetrad_twist(x) * i;

    return result;
}

static long long get_tw_g3(cube x)
{
    long long result=0, i=1;

    result += get_permutation(x.tetrads[TETRAD_URF], 4);
    i *= fact[4];

    result += get_permutation(x.slices[SLICE_UD], 4) * i;
    i *= fact[4];

    for (int i=0; i<4; ++i) *(x.slices[SLICE_RL]+i) -= 4;
    result += get_permutation(x.slices[SLICE_RL], 4) * i;
    i *= fact[4];

    for (int i=0; i<4; ++i) *(x.slices[SLICE_FB]+i) -= 8;
    result += get_partial_permutation(x.slices[SLICE_FB], 4, 2) * i;
    i *= pick(4, 2);

    for (int i=0; i<4; ++i) *(x.tetrads[TETRAD_URB]+i) -= 4;
    result += get_partial_permutation(x.tetrads[TETRAD_URB], 4, 1) * i;

    return result;
}

static cube set_tw_g0(long long result)
{
    cube x = new_cube();
    long long i;

    i = result;
    set_eo(&x, i);

    return x;
}

static cube set_tw_g1(long long result)
{
    cube x = new_cube();
    long long i;

    i = result%pow3[NUM_CORNERS-1];
    set_co(&x, i);
    result /= pow3[NUM_CORNERS-1];

    i = result;
    set_combination(x.slices[SLICE_UD], 12, 4, i);

    return x;
}

static cube set_tw_g2(long long result)
{
    cube x = new_cube();
    long long i;

    i = result%choose(8, 4);
    set_combination(x.slices[SLICE_RL], 8, 4, i);
    for (int i=0; i<8; ++i) *(x.slices[SLICE_RL]+i) += 4;
    result /= choose(8, 4);

    i = result%choose(8, 4);
    set_combination(x.tetrads[TETRAD_URF], 8, 4, i);
    result /= choose(8, 4);

    i = result;
    set_tetrad_twist(&x, i);

    return x;
}

static cube set_tw_g3(long long result)
{
    cube x = new_cube();
    long long i;

    i = result%fact[4];
    set_permutation(x.tetrads[TETRAD_URF], 4, i);
    result /= fact[4];

    i = result%fact[4];
    set_permutation(x.slices[SLICE_UD], 4, i);
    result /= fact[4];

    i = result%fact[4];
    set_permutation(x.slices[SLICE_RL], 4, i);
    for (int i=0; i<4; ++i) *(x.slices[SLICE_RL]+i) += 4;
    result /= fact[4];

    i = result%pick(4, 2);
    set_partial_permutation(x.slices[SLICE_FB], 4, 2, i);
    for (int i=0; i<4; ++i) *(x.slices[SLICE_FB]+i) += 8;
    result /= pick(4, 2);

    i = result;
    set_partial_permutation(x.tetrads[TETRAD_URB], 4, 1, i);
    for (int i=0; i<4; ++i) *(x.tetrads[TETRAD_URB]+i) += 4;

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

static coord tw_coords[] = 
{
    {.name="tw_g0", .get=get_tw_g0, .set=set_tw_g0, .h=h_tw_g0, .quater_turns={1, 1, 1, 1, 1, 1}, .order=2048},
    {.name="tw_g1", .get=get_tw_g1, .set=set_tw_g1, .h=h_tw_g1, .quater_turns={1, 1, 0, 1, 1, 0}, .order=1082565},
    {.name="tw_g2", .get=get_tw_g2, .set=set_tw_g2, .h=h_tw_g2, .quater_turns={1, 0, 0, 1, 0, 0}, .order=29400},
    {.name="tw_g3", .get=get_tw_g3, .set=set_tw_g3, .h=h_tw_g3, .quater_turns={0, 0, 0, 0, 0, 0}, .order=663552},
};
