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

static long long get_cp(cube x)
{
    long long result=0;

    result += get_permutation(x.corners, 8);

    return result;
}

static cube set_cp(long long result)
{
    cube x = new_cube();
    long long i;

    i = result;
    set_permutation(x.corners, 8, i);

    return x;
}

////////////////////////////////////////////////////////////////////////////////

static long long get_tw_g0(cube x)
{
    long long result=0;

    result += get_eo(x);

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

static int h_tw_g0(cube x)
{
    return table_get(tw_coords[0].table, tw_coords[0].get(x));
}

static long long get_tw_g1(cube x)
{
    long long result=0, i=1;

    result += get_co(x);
    i *= pow3[NUM_CORNERS-1];

    result += get_combination(x.slices[SLICE_UD], 12, 4) * i;

    return result;
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

static int h_tw_g1(cube x)
{
    return table_get(tw_coords[1].table, tw_coords[1].get(x));
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

static int h_tw_g2(cube x)
{
    return table_get(tw_coords[2].table, tw_coords[2].get(x));
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

static int h_tw_g3(cube x)
{
    return table_get(tw_coords[3].table, tw_coords[3].get(x));
}

static coord tw_coords[] =
{
    {
        .name = "tw_g0",
        .get = get_tw_g0,
        .set = set_tw_g0,
        .h = h_tw_g0,
        .order = 2048,
    },
    {
        .name = "tw_g1",
        .get = get_tw_g1,
        .set = set_tw_g1,
        .h = h_tw_g1,
        .order = 1082565,
        .move_mask = EO_MASK,
    },
    {
        .name = "tw_g2",
        .get = get_tw_g2,
        .set = set_tw_g2,
        .h = h_tw_g2,
        .order = 29400,
        .move_mask = DR_MASK,
    },
    {
        .name = "tw_g3",
        .get = get_tw_g3,
        .set = set_tw_g3,
        .h = h_tw_g3,
        .order = 663552,
        .move_mask = HTR_MASK,
    },
};

////////////////////////////////////////////////////////////////////////////////

static long long get_ko_g0(cube x)
{
    long long result=0, i=1;

    result = get_flip_ud_slice(x);
    x = apply_sym(x, flip_ud_slice_coord_to_rep_sym[result]);
    result = flip_ud_slice_coord_to_eqv_class[result];
    i *= 64430;

    result += get_co(x) * i;

    return result;
}

static cube set_ko_g0(long long result)
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

static int h_ko_g0(cube x)
{
    return table_get(ko_coords[0].table, ko_coords[0].get(x));
}

static long long get_ko_g1(cube x)
{
    long long result=0, i=1;

    result = get_cp(x);
    x = apply_sym(x, cp_coord_to_rep_sym[result]);
    result = cp_coord_to_eqv_class[result];
    i *= 2768;

    for (int i=0; i<8; ++i) *(x.slices[SLICE_RL]+i) -= 4;
    result += get_permutation(x.slices[SLICE_RL], 8) * i;

    return result;
}

static cube set_ko_g1(long long result)
{
    cube x = new_cube();
    long long i;

    i = cp_eqv_class_to_rep[result%2768];
    x = set_cp(i);
    result /= 2768;

    i = result;
    set_permutation(x.slices[SLICE_RL], 8, i);
    for (int i=0; i<8; ++i) *(x.slices[SLICE_RL]+i) += 4;

    return x;
}

static int h_ko_g1(cube x)
{
    return table_get(ko_coords[1].table, ko_coords[1].get(x));
}

static coord ko_coords[] =
{
    {
        .name = "ko_g0",
        .get = get_ko_g0,
        .set = set_ko_g0,
        .h = h_ko_g0,
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
        .name = "ko_g1",
        .get = get_ko_g1,
        .set = set_ko_g1,
        .h = h_ko_g1,
        .order = 111605760,
        .move_mask = DR_MASK,
        .is_sym = 1,
        .num_syms = 16,
        .eqv_classes = 2768,
        .self_syms = cp_self_syms,
        .coord_to_rep = cp_coord_to_rep,
        .coord_to_rep_sym = cp_coord_to_rep_sym,
        .coord_to_eqv_class = cp_coord_to_eqv_class,
        .eqv_class_to_rep = cp_eqv_class_to_rep,
        .get_sym_part = get_cp,
        .set_sym_part = set_cp,
        .sym_part_order = 40320,
    },
};
