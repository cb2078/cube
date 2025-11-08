static long long get_flip_ud_slice(cube x)
{
    long long result=0, i=1;

    result += get_eo(x);
    i *= pow2[NUM_EDGES-1];

    for (int i=0; i<20; ++i) x.cubies[i] &= 0x0f;

    result += get_combination(x.slices[SLICE_UD], 12, 4) * i;

    return result;
}

static cube set_flip_ud_slice(long long result)
{
    cube x = new_cube();
    long long i;

    i = result%pow2[NUM_EDGES-1];
    set_eo(&x, i);
    result /= pow2[NUM_EDGES-1];

    cube y = x;
    x = new_cube();

    i = result;
    set_combination(x.slices[SLICE_UD], 12, 4, i);

    x = compose(x, y);
    return x;
}

static long long get_tw_g0(cube x)
{
    long long result=0, i=1;

    result = get_flip_ud_slice(x);
    x = apply_sym(x, flip_ud_slice_coord_to_rep_sym[result]);
    result = flip_ud_slice_coord_to_eqv_class[result];
    i *= 64430;

    result += get_co(x) * i;

    return result;
}

static cube set_tw_g0(long long result)
{
    cube x = new_cube();
    long long i;

    i = flip_ud_slice_eqv_class_to_rep[result%64430];
    x = set_flip_ud_slice(i);
    result /= 64430;

    i = result;
    set_co(&x, i);

    return x;
}

static int h_tw_g0(cube x)
{
    return table_get(tw_coords[0].table, tw_coords[0].get(x));
}

static long long get_tw_g1(cube x)
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

static cube set_tw_g1(long long result)
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

static int h_tw_g1(cube x)
{
    return table_get(tw_coords[1].table, tw_coords[1].get(x));
}

static long long get_tw_g2(cube x)
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

static cube set_tw_g2(long long result)
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

static int h_tw_g2(cube x)
{
    return table_get(tw_coords[2].table, tw_coords[2].get(x));
}

static coord tw_coords[] =
{
    {
        .name = "tw_g0",
        .get = get_tw_g0,
        .set = set_tw_g0,
        .h = h_tw_g0,
        .quater_turns = {1, 1, 1, 1, 1, 1},
        .order = 140908410,
        .is_sym = 1,
        .num_syms = 16,
        .eqv_classes = 64430,
        .self_syms = flip_ud_slice_self_syms,
        .coord_to_rep = flip_ud_slice_coord_to_rep,
        .coord_to_rep_sym = flip_ud_slice_coord_to_rep_sym,
        .coord_to_eqv_class = flip_ud_slice_coord_to_eqv_class,
        .eqv_class_to_rep = flip_ud_slice_eqv_class_to_rep,
        .get_sym_part = get_flip_ud_slice,
        .set_sym_part = set_flip_ud_slice,
        .sym_part_order = 1013760,
    },
    {
        .name = "tw_g1",
        .get = get_tw_g1,
        .set = set_tw_g1,
        .h = h_tw_g1,
        .quater_turns = {1, 0, 0, 1, 0, 0},
        .order = 29400,
    },
    {
        .name = "tw_g2",
        .get = get_tw_g2,
        .set = set_tw_g2,
        .h = h_tw_g2,
        .quater_turns = {0, 0, 0, 0, 0, 0},
        .order = 663552,
    },
};
