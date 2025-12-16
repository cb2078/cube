#define COORD(NAME, SYM, SYM_MAX, SYM_CLASSES, BASIC, BASIC_MAX)\
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

COORD(eo_none,
      csep, CSEP_MAX, 9,
      esep, ESEP_MAX);

COORD(eo_partial,
      csep, CSEP_MAX, 9,
      partial_eo_esep, PARTIAL_EO_ESEP_MAX);

COORD(eo_full,
      csep, CSEP_MAX, 9,
      eo_esep, EO_ESEP_MAX);

////////////////////////////////////////////////////////////////////////////////

static void init_coord(struct coord *c)
{
    static int initialised = 0;
    if (initialised)
        return;

    if (c->raw.max == 0)
    {
        c->raw.max = PARTIAL_EO_ESEP_MAX;
        c->max = c->sym.classes * c->raw.max;
    }
#ifdef DEBUG
    if (c->sym.classes == 0)
        c->sym.classes = c->sym.max;
#else
    ASSERT(c->sym.classes);
#endif
    snprintf(c->filename, sizeof(c->filename), "e%d.bin", EO_VARIANT);

    FILE *fp;
    c->to_rep = malloc(sizeof(int)*c->sym.classes);
    c->to_class = malloc(sizeof(int)*c->sym.max);
    c->to_sym = malloc(sizeof(int)*c->sym.max);
    c->table = table_new(c->max, 4);
    if (!NO_INPUT && (fp = fopen(c->filename, "rb")))
    {
        fread(c->to_rep, sizeof(int)*c->sym.classes, 1, fp);
        fread(c->to_class, sizeof(int)*c->sym.max, 1, fp);
        fread(c->to_sym, sizeof(int)*c->sym.max, 1, fp);
        fread(c->table->data, c->table->size, 1, fp);
        LOG("read '%s'\n", c->filename);
    }
    else
        if ((fp = fopen(c->filename, "wb")))
    {
        init_sym(c);
        init_prune_table(c);
        fwrite(c->to_rep, sizeof(int)*c->sym.classes, 1, fp);
        fwrite(c->to_class, sizeof(int)*c->sym.max, 1, fp);
        fwrite(c->to_sym, sizeof(int)*c->sym.max, 1, fp);
        fwrite(c->table->data, c->table->size, 1, fp);
        LOG("wrote '%s'\n", c->filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", c->filename);
    }
    fclose(fp);
    initialised = 1;
}
