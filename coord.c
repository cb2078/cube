#define COORD(NAME, MASK, SYM, SYM_MAX, SYM_CLASSES, BASIC, BASIC_MAX)\
    \
    static long long get_##NAME(cube_t);\
    static cube_t set_##NAME(long long);\
    static int h_##NAME(cube_t);\
    \
    static struct coord coord_##NAME =\
    {\
        .filename = #NAME".bin",\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .h = h_##NAME,\
        .max = (SYM_CLASSES)*(BASIC_MAX),\
        .move_mask = MASK,\
        .num_syms = 48,\
        .sym =\
        {\
            .get = get_##SYM,\
            .set = set_##SYM,\
            .max = SYM_MAX,\
            .classes = SYM_CLASSES,\
        },\
        .raw =\
        {\
            .get = get_##BASIC,\
            .set = set_##BASIC,\
            .max = BASIC_MAX,\
        },\
    };\
    \
    static long long get_##NAME(cube_t x)\
    {\
        return get_sym_comp(x, &coord_##NAME);\
    }\
    \
    static cube_t set_##NAME(long long x)\
    {\
        return set_sym_comp(x, &coord_##NAME);\
    }\
    \
    static int h_##NAME(cube_t x)\
    {\
        return table_get(coord_##NAME.table, get_##NAME(x));\
    }

static long long get_sym_comp(cube_t x, struct coord *c)
{
    long long r = c->sym.get(x);
    x = apply_sym(x, c->to_sym[r]);
    r = c->to_class[r];
    return r * c->raw.max + c->raw.get(x);
}

static cube_t set_sym_comp(long long r, struct coord *c)
{
    cube_t x = c->sym.set(c->to_rep[r/c->raw.max]);
    cube_t y = c->raw.set(r%c->raw.max);
    return compose(x, y);
}

////////////////////////////////////////////////////////////////////////////////

static long long get_twist_corner_sep(cube_t x)
{
    return get_twist(x) * CORNER_SEP_MAX + get_corner_sep(x);
}

static cube_t set_twist_corner_sep(long long r)
{
    cube_t x = set_corner_sep(r%CORNER_SEP_MAX);
    cube_t y = set_twist(r/CORNER_SEP_MAX);
    return compose(x, y);
}

static long long get_flip_edge_sep(cube_t x)
{
    return (get_flip(x) & (PARTIAL_FLIP_MAX-1)) * EDGE_SEP_MAX + get_edge_sep(x);
}

static cube_t set_flip_edge_sep(long long r)
{
    cube_t x = set_edge_sep(r%EDGE_SEP_MAX);
    cube_t y = set_flip(r/EDGE_SEP_MAX);
    return compose(x, y);
}

COORD(optimal, 0,
      corner_sep, CORNER_SEP_MAX, 9,
      // twist_corner_sep, TWIST_MAX*CORNER_SEP_MAX, 3393,
      flip_edge_sep, PARTIAL_FLIP_MAX*EDGE_SEP_MAX);
