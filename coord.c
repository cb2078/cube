#define COORD(NAME, MASK, SYM, SYM_MAX, SYM_CLASSES, BASIC, BASIC_MAX)\
    \
    static long long get_##NAME(cube_t);\
    static cube_t set_##NAME(long long);\
    static int h_##NAME(cube_t);\
    static int h_##NAME##_optimal(cube_t);\
    \
    static struct coord coord_##NAME =\
    {\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .h = h_##NAME,\
        .h_optimal = h_##NAME##_optimal,\
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
    }\
    \
    static int h_##NAME##_optimal(cube_t x)\
    {\
        return h_##NAME(x) ?: !cube_eq(x, new_cube());\
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

static long long get_co_csep(cube_t x)
{
    return get_co(x) * CSEP_MAX + get_csep(x);
}

static cube_t set_co_csep(long long r)
{
    cube_t x = set_csep(r%CSEP_MAX);
    cube_t y = set_co(r/CSEP_MAX);
    return compose(x, y);
}

static long long get_eo_esep(cube_t x)
{
    return get_eo(x) * ESEP_MAX + get_esep(x);
}

static cube_t set_eo_esep(long long r)
{
    cube_t x = set_esep(r%ESEP_MAX);
    cube_t y = set_eo(r/ESEP_MAX);
    return compose(x, y);
}

static long long get_partial_eo_esep(cube_t x)
{
    return (get_eo(x) & (PARTIAL_EO_MAX-1)) * ESEP_MAX + get_esep(x);
}

static cube_t set_partial_eo_esep(long long r)
{
    return set_eo_esep(r);
}

// TODO remove the move mask parameter in coordinate macro (since it's always 0)
COORD(eo_none, 0,
      csep, CSEP_MAX, 9,
      esep, ESEP_MAX);

COORD(eo_partial, 0,
      csep, CSEP_MAX, 9,
      partial_eo_esep, PARTIAL_EO_ESEP_MAX);

COORD(eo_full, 0,
      csep, CSEP_MAX, 9,
      eo_esep, EO_ESEP_MAX);
