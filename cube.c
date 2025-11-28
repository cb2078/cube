static cube_t new_cube(void)
{
    cube_t x;
    for (int i=0; i<NUM_CORNERS; ++i) x.corners[i]=i;
    for (int i=0; i<NUM_EDGES; ++i) x.edges[i]=i;
    return x;
}

static void print_cube(cube_t x)
{
    #define PRINT(x, ...) \
        printf("%2s ", x); \
        for (int i=0; i<NUM_CORNERS+NUM_EDGES; ++i) \
            printf(__VA_ARGS__); \
        printf("\n");
    PRINT("",   "%s%3s", i?" ":"", cubie_str[i]);
    PRINT("p:", "%s%3d", i?",":"", x.cubies[i]&0x0f);
    PRINT("o:", "%s%3d", i?",":"", x.cubies[i]>>4);
    #undef PRINT
}

static int cube_eq(cube_t x, cube_t y)
{
    return 0==memcmp(&x, &y, sizeof(cube_t));
}

static cube_t compose(cube_t x, cube_t y)
{
    cube_t z;
    for (int i=0; i<NUM_CORNERS; ++i)
    {
        z.corners[i] = x.corners[y.corners[i]&0x0f];
        z.corners[i] += y.corners[i]&0xf0;
        z.corners[i] %= 3*0x10;
    }
    for (int i=0; i<NUM_EDGES; ++i)
    {
        z.edges[i] = x.edges[y.edges[i]&0x0f];
        z.edges[i] += y.edges[i]&0xf0;
        z.edges[i] %= 2*0x10;
    }
    return z;
}

static cube_t compose_3(cube_t x, cube_t y, cube_t z)
{
    return compose(compose(x, y), z);
}

static cube_t apply_sym(cube_t x, int sym)
{
    // TODO try creating a "mirroed compose" function to see if its faster than
    // inverting the CO twice

    cube_t maybe_invert_co(cube_t x)
    {
        return sym&1 ? invert_co(x) : x;
    }

    return compose_3(sym_cubes[inv_sym[sym]], maybe_invert_co(x), maybe_invert_co(sym_cubes[sym]));
}

static cube_t apply_move(cube_t x, int move)
{
    return compose(x, move_cubes[move]);
}

static cube_t apply_moves(cube_t x, int *moves, int length)
{
    for (int i=0; i<length; ++i)
        x = apply_move(x, moves[i]);
    return x;
}

static void orient(char *x, int orientation)
{
    assert(orientation<3);
    *x = (*x&0x0f) | (orientation<<4);
}

static long long get_flip(cube_t x)
{
    int r = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
        r += x.edges[i]>>4<<i;
    return r;
}

static cube_t set_flip(long long r)
{
    cube_t x = new_cube();
    int parity = 0;
    for (int i=0, y; i<NUM_EDGES-1; ++i, r>>=1)
        orient(x.edges+i, y=r&1), parity^=y;
    orient(x.edges+NUM_EDGES-1, parity);
    return x;
}

static long long get_twist(cube_t x)
{
    int r = 0;
    for (int i=0; i<NUM_CORNERS-1; ++i)
        r += (x.corners[i]>>4)*pow3[i];
    return r;
}

static cube_t set_twist(long long r)
{
    cube_t x = new_cube();
    int parity = 0;
    for (int i=0, y; i<NUM_CORNERS-1; ++i, r/=3)
        orient(x.corners+i, y=r%3), parity+=y;
    orient(x.corners+NUM_CORNERS-1, (3-parity%3)%3);
    return x;
}

static long long get_corner_sep(cube_t x)
{
    for (int i=0; i<8; ++i) x.corners[i] &= 0x0f;
    return get_combination(x.corners, 8, 4);
}

static cube_t set_corner_sep(long long r)
{
    cube_t x = new_cube();
    set_combination(x.corners, 8, 4, r);
    return x;
}

static long long get_edge_sep(cube_t x)
{
    for (int i=0; i<12; ++i) x.edges[i] &= 0x0f;
    cube_t y = x;
    long long r = get_combination(x.edges, 12, 4);
    for (int i=0, j=0; i<12; ++i)
        x.edges[i]<4 ? ++j : (y.edges[i-j] = x.edges[i]-4);
    return r * 70 + get_combination(y.edges, 8, 4);
}

static cube_t set_edge_sep(long long r)
{
    cube_t x, y, z;
    x = y = z = new_cube();
    set_combination(x.edges, 12, 4, r/70);
    set_combination(y.edges, 8, 4, r%70);
    for (int i=0, j=0; i<12; ++i)
        z.edges[i] = x.edges[i]<4 ? (++j, x.edges[i]) : y.edges[i-j]+4;
    return z;
}

static cube_t invert_co(cube_t x)
{
    for (int i=0; i<NUM_CORNERS; ++i)
    {
        int twist = x.corners[i]>>4;
        orient(x.corners+i, twist?3-twist:twist);
    }
    return x;
}
