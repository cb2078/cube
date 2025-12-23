#define RAW_COORD(NAME, MAX)\
    \
    static struct raw_coord raw_coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = MAX,\
    };

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

RAW_COORD(esep, ESEP_MAX);
RAW_COORD(partial_eo_esep, 0);
RAW_COORD(eo_esep, EO_ESEP_MAX);

////////////////////////////////////////////////////////////////////////////////

#define SYM_COORD(NAME, MAX, CLASSES)\
    \
    static struct sym_coord sym_coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = MAX,\
        .classes = CLASSES,\
    };

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

SYM_COORD(csep, CSEP_MAX, 9);
SYM_COORD(co_csep, CO_CSEP_MAX, 3393);

////////////////////////////////////////////////////////////////////////////////

#define COORD(NAME, SYM, SYM_CLASSES, RAW, RAW_MAX)\
    \
    static long long get_##NAME(cube_t);\
    static cube_t set_##NAME(long long);\
    static int h_##NAME(cube_t);\
    static int h_##NAME##_optimal(cube_t);\
    \
    static struct coord coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = SYM_CLASSES * RAW_MAX,\
        .h = h_##NAME,\
        .h_optimal = h_##NAME##_optimal,\
        .raw = &raw_coord_##RAW,\
        .sym = &sym_coord_##SYM,\
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
        long long i = get_##NAME(x);\
        int r = table_get(coord_##NAME.table, i);\
        return r ? r+PRUNE_BASE\
            : (table_get(coord_##NAME.table, PRUNE_MIN_62(i)) +\
               4*table_get(coord_##NAME.table, PRUNE_MIN_62(i)+1));\
    }\
    \
    static int h_##NAME##_optimal(cube_t x)\
    {\
        return h_##NAME(x) ?: !cube_eq(x, new_cube());\
    }

static long long get_sym_comp(cube_t x, struct coord *c)
{
    long long r = c->sym->get(x);
    x = apply_sym(x, c->sym->to_sym[r]);
    r = c->sym->to_class[r];
    return r * c->raw->max + c->raw->get(x);
}

static cube_t set_sym_comp(long long r, struct coord *c)
{
    cube_t x = c->sym->set(c->sym->to_rep[r/c->raw->max]);
    cube_t y = c->raw->set(r%c->raw->max);
    return compose(x, y);
}

#if 0
COORD(eo_none, csep, 9, esep, ESEP_MAX);
COORD(eo_partial, csep, 9, partial_eo_esep, 0);
COORD(eo_full, csep, 9, eo_esep, EO_ESEP_MAX);
#else
COORD(eo_none, co_csep, 3393, esep, ESEP_MAX);
COORD(eo_partial, co_csep, 3393, partial_eo_esep, 0);
COORD(eo_full, co_csep, 3393, eo_esep, EO_ESEP_MAX);
#endif

////////////////////////////////////////////////////////////////////////////////

static void init_coord(struct coord *c)
{
    static int initialised = 0;
    if (initialised)
        return;

    c->raw->max = PARTIAL_EO_ESEP_MAX;
    c->max = c->sym->classes * c->raw->max;
#ifdef DEBUG
    if (c->sym->classes == 0)
        c->sym->classes = c->sym->max;
#else
    ASSERT(c->sym->classes);
#endif
    ASSERT(c->max);

    char filename[256];
    snprintf(filename, sizeof(filename), "e%d.bin", EO_VARIANT);
    FILE *fp;
    c->sym->to_rep = malloc(sizeof(int)*c->sym->classes);
    c->sym->to_class = malloc(sizeof(int)*c->sym->max);
    c->sym->to_sym = malloc(sizeof(int)*c->sym->max);
    c->table = table_new(c->max, 2);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(c->sym->to_rep, sizeof(int)*c->sym->classes, 1, fp);
        fread(c->sym->to_class, sizeof(int)*c->sym->max, 1, fp);
        fread(c->sym->to_sym, sizeof(int)*c->sym->max, 1, fp);
        fread(c->table->data, c->table->size, 1, fp);
        LOG("read '%s'\n", filename);
    }
    else
        if ((fp = fopen(filename, "wb")))
    {
        init_sym(c->sym);
        init_prune_table(c);
        fwrite(c->sym->to_rep, sizeof(int)*c->sym->classes, 1, fp);
        fwrite(c->sym->to_class, sizeof(int)*c->sym->max, 1, fp);
        fwrite(c->sym->to_sym, sizeof(int)*c->sym->max, 1, fp);
        fwrite(c->table->data, c->table->size, 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
    initialised = 1;
}
