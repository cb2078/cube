static long long get_flip(cube_t x)
{
    long long r = 0;

    r += get_eo(x);

    return r;
}

static cube_t set_flip(long long r)
{
    cube_t x = new_cube();

    set_eo(&x, r);

    return x;
}

static int h_flip(cube_t x)
{
    return table_get(thistlethwaite_coords[0].table, thistlethwaite_coords[0].get(x));
}

static long long get_stage2(cube_t x)
{
    long long r = 0;

    r += get_co(x);

    r *= choose[12][4];
    r += get_combination(x.edges, 12, 4);

    return r;
}

static cube_t set_stage2(long long r)
{
    cube_t x = new_cube();

    set_combination(x.edges, 12, 4, r%choose[12][4]);
    r /= choose[12][4];

    set_co(&x, r);

    return x;
}

static int h_stage2(cube_t x)
{
    return table_get(thistlethwaite_coords[1].table, thistlethwaite_coords[1].get(x));
}

static long long get_stage3(cube_t x)
{
    long long r = 0;

    for (int i=4; i<12; ++i) x.edges[i] -= 4;
    r += get_combination(x.edges+4, 8, 4);

    r *= choose[8][4];
    r += get_combination(x.corners, 8, 4);

    return r;
}

static cube_t set_stage3(long long r)
{
    cube_t x = new_cube();

    set_combination(x.corners, 8, 4, r%choose[8][4]);
    r /= choose[8][4];

    set_combination(x.edges+4, 8, 4, r);
    for (int i=4; i<12; ++i) x.edges[i] += 4;

    return x;
}

static int h_stage3(cube_t x)
{
    return table_get(thistlethwaite_coords[2].table, thistlethwaite_coords[2].get(x));
}

static long long get_stage4(cube_t x)
{
    long long r = 0;

    r += get_permutation(x.urf_tetrad, 4);

    for (int i=0; i<4; ++i) x.urb_tetrad[i] -= 4;
    r *= pick[4][1];
    r += get_partial_permutation(x.urb_tetrad, 4, 1);

    r *= fact[4];
    r += get_permutation(x.ud_slice, 4);

    for (int i=0; i<4; ++i) x.rl_slice[i] -= 4;
    r *= fact[4];
    r += get_permutation(x.rl_slice, 4);

    for (int i=0; i<4; ++i) x.fb_slice[i] -= 8;
    r *= pick[4][2];
    r += get_partial_permutation(x.fb_slice, 4, 2);

    return r;
}

static cube_t set_stage4(long long r)
{
    cube_t x = new_cube();

    set_partial_permutation(x.fb_slice, 4, 2, r%pick[4][2]);
    r /= pick[4][2];
    for (int i=0; i<4; ++i) x.fb_slice[i] += 8;

    set_permutation(x.rl_slice, 4, r%fact[4]);
    r /= fact[4];
    for (int i=0; i<4; ++i) x.rl_slice[i] += 4;

    set_permutation(x.ud_slice, 4, r%fact[4]);
    r /= fact[4];

    set_partial_permutation(x.urb_tetrad, 4, 1, r%pick[4][1]);
    r /= pick[4][1];
    for (int i=0; i<4; ++i) x.urb_tetrad[i] += 4;

    set_permutation(x.urf_tetrad, 4, r);

    return x;
}

static int h_stage4(cube_t x)
{
    return table_get(thistlethwaite_coords[3].table, thistlethwaite_coords[3].get(x));
}

static struct coord thistlethwaite_coords[] =
{
    {
        .name = "flip",
        .get = get_flip,
        .set = set_flip,
        .max = 2048,
        .h = h_flip,
    },
    {
        .name = "stage2",
        .get = get_stage2,
        .set = set_stage2,
        .max = 1082565,
        .h = h_stage2,
        .move_mask = EO_MASK,
    },
    {
        .name = "stage3",
        .get = get_stage3,
        .set = set_stage3,
        .max = 4900,
        .h = h_stage3,
        .move_mask = DR_MASK,
    },
    {
        .name = "stage4",
        .get = get_stage4,
        .set = set_stage4,
        .max = 663552,
        .h = h_stage4,
        .move_mask = HTR_MASK,
    },
};

////////////////////////////////////////////////////////////////////////////////

// TODO read/write these to disk
static int flip_ud_slice_to_rep[64430];
static int flip_ud_slice_to_class[1013760];
static int flip_ud_slice_to_sym[1013760];
static int flip_ud_slice_self_syms[1013760];

static long long get_flip_ud_slice(cube_t x)
{
    long long r = 0;

    r += get_eo(x);

    set_eo(&x, 0);
    r *= choose[12][4];
    r += get_combination(x.edges, 12, 4);

    return r;
}

static cube_t set_flip_ud_slice(long long r)
{
    cube_t x = new_cube();

    set_combination(x.edges, 12, 4, r%choose[12][4]);
    r /= choose[12][4];

    set_eo(&x, r);

    return x;
}

static long long get_phase1(cube_t x)
{
    long long r = 0;

    r += get_flip_ud_slice(x);
    x = apply_sym(x, flip_ud_slice_to_sym[r]);
    r = flip_ud_slice_to_class[r];

    r *= pow3[7];
    r += get_co(x);

    return r;
}

static cube_t set_phase1(long long r)
{
    cube_t x;

    x = set_flip_ud_slice(flip_ud_slice_to_rep[r/2187]);
    r %= 2187;

    set_co(&x, r);

    return x;
}

static int h_phase1(cube_t x)
{
    return table_get(kociemba_coords[0].table, kociemba_coords[0].get(x));
}

// TODO read/write these to disk
static int corner_perm_to_rep[2768];
static int corner_perm_to_class[40320];
static int corner_perm_to_sym[40320];
static int corner_perm_self_syms[40320];

static long long get_corner_perm(cube_t x)
{
    long long r = 0;

    r += get_permutation(x.corners, 8);

    return r;
}

static cube_t set_corner_perm(long long r)
{
    cube_t x = new_cube();

    set_permutation(x.corners, 8, r);

    return x;
}

static long long get_phase2(cube_t x)
{
    long long r = 0;

    r += get_corner_perm(x);
    x = apply_sym(x, corner_perm_to_sym[r]);
    r = corner_perm_to_class[r];

    for (int i=4; i<12; ++i) x.edges[i] -= 4;
    r *= fact[8];
    r += get_permutation(x.edges+4, 8);

    return r;
}

static cube_t set_phase2(long long r)
{
    cube_t x;

    x = set_corner_perm(corner_perm_to_rep[r/40320]);
    r %= 40320;

    set_permutation(x.edges+4, 8, r);
    for (int i=4; i<12; ++i) x.edges[i] += 4;

    return x;
}

static int h_phase2(cube_t x)
{
    return table_get(kociemba_coords[1].table, kociemba_coords[1].get(x));
}

static struct coord kociemba_coords[] =
{
    {
        .name = "phase1",
        .get = get_phase1,
        .set = set_phase1,
        .max = 140908410,
        //
        .num_syms = 16,
        .classes = 64430,
        .to_rep = flip_ud_slice_to_rep,
        .to_class = flip_ud_slice_to_class,
        .to_sym = flip_ud_slice_to_sym,
        .self_syms = flip_ud_slice_self_syms,
        .base = &(struct coord)
        {
            .name = "flip_ud_slice",
            .get = get_flip_ud_slice,
            .set = set_flip_ud_slice,
            .max = 1013760,
        },
        .h = h_phase1,
    },
    {
        .name = "phase2",
        .get = get_phase2,
        .set = set_phase2,
        .max = 111605760,
        //
        .num_syms = 16,
        .classes = 2768,
        .to_rep = corner_perm_to_rep,
        .to_class = corner_perm_to_class,
        .to_sym = corner_perm_to_sym,
        .self_syms = corner_perm_self_syms,
        .base = &(struct coord)
        {
            .name = "corner_perm",
            .get = get_corner_perm,
            .set = set_corner_perm,
            .max = 40320,
        },
        .h = h_phase2,
        .move_mask = DR_MASK,
    },
};

////////////////////////////////////////////////////////////////////////////////

// TODO read/write these to disk
static int ud_slice_sorted_to_rep[788];
static int ud_slice_sorted_to_class[11880];
static int ud_slice_sorted_to_sym[11880];
static int ud_slice_sorted_self_syms[11880];

static long long get_ud_slice_sorted(cube_t x)
{
    long long r = 0;

    set_eo(&x, 0);
    r += get_partial_permutation(x.edges, 12, 4);

    return r;
}

static cube_t set_ud_slice_sorted(long long r)
{
    cube_t x = new_cube();

    set_partial_permutation(x.edges, 12, 4, r);

    return x;
}

static long long get_optimal(cube_t x)
{
    long long r = 0;

    r += get_ud_slice_sorted(x);
    x = apply_sym(x, ud_slice_sorted_to_sym[r]);
    r = ud_slice_sorted_to_class[r];

    r *= pow2[11];
    r += get_eo(x);

    r *= pow3[7];
    r += get_co(x);

    return r;
}

static cube_t set_optimal(long long r)
{
    cube_t x;

    x = set_ud_slice_sorted(ud_slice_sorted_to_rep[r/4478976]);
    r %= 4478976;

    set_co(&x, r%pow3[7]);
    r /= pow3[7];

    set_eo(&x, r);

    return x;
}

static int h_optimal(cube_t x)
{
    return table_get(optimal_coords[0].table, optimal_coords[0].get(x));
}

static struct coord optimal_coords[] =
{
    {
        .name = "optimal",
        .get = get_optimal,
        .set = set_optimal,
        .max = 3529433088,
        //
        .num_syms = 16,
        .classes = 788,
        .to_rep = ud_slice_sorted_to_rep,
        .to_class = ud_slice_sorted_to_class,
        .to_sym = ud_slice_sorted_to_sym,
        .self_syms = ud_slice_sorted_self_syms,
        .base = &(struct coord)
        {
            .name = "ud_slice_sorted",
            .get = get_ud_slice_sorted,
            .set = set_ud_slice_sorted,
            .max = 11880,
        },
        .h = h_optimal,
    },
};
