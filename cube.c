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
    cube_t result;

    for (int i=0; i<NUM_CORNERS; ++i)
    {
        result.corners[i] = x.corners[y.corners[i]&0x0f];
        result.corners[i] += y.corners[i]&0xf0;
        result.corners[i] %= 3*0x10;
    }

    for (int i=0; i<NUM_EDGES; ++i)
    {
        result.edges[i] = x.edges[y.edges[i]&0x0f];
        result.edges[i] += y.edges[i]&0xf0;
        result.edges[i] %= 2*0x10;
    }

    return result;
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
    cube_t result=x;
    for (int i=0; i<length; ++i) result=apply_move(result, moves[i]);
    return result;
}

static void orient(char *x, int orientation)
{
    assert(orientation<3);
    *x = (*x&0x0f) | (orientation<<4);
}

static long long get_eo(cube_t x)
{
    int result = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
        result += x.edges[i]>>4<<i;
    return result;
}

static void set_eo(cube_t *x, long long r)
{
    int parity = 0;
    for (int i=0, y; i<NUM_EDGES-1; ++i, r>>=1)
        orient(x->edges+i, y=r&1), parity^=y;
    orient(x->edges+NUM_EDGES-1, parity);
}

static long long get_co(cube_t x)
{
    int result = 0;
    for (int i=0; i<NUM_CORNERS-1; ++i)
        result += (x.corners[i]>>4)*pow3[i];
    return result;
}

static void set_co(cube_t *x, long long r)
{
    int parity = 0;
    for (int i=0, y; i<NUM_CORNERS-1; ++i, r/=3)
        orient(x->corners+i, y=r%3), parity+=y;
    orient(x->corners+NUM_CORNERS-1, (3-parity%3)%3);
}

static cube_t invert_co(cube_t x)
{
    for (int i=0; i<NUM_CORNERS; ++i)
    {
        int co = x.corners[i]>>4;
        orient(x.corners+i, co?3-co:co);
    }
    return x;
}
