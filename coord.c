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
    return (get_eo(x) & (PARTIAL_EO_MAX-1)) * ESEP_MAX + get_esep(x);
}

static cube_t set_eo_esep(long long r)
{
    cube_t x = set_esep(r%ESEP_MAX);
    cube_t y = set_eo(r/ESEP_MAX);
    return compose(x, y);
}

COORD(partial_eo, 0,
      csep, CSEP_MAX, 9,
      // co_csep, CO_MAX*CSEP_MAX, 3393,
      eo_esep, PARTIAL_EO_MAX*ESEP_MAX);
