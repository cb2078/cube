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

#define set_partial_eo_esep set_eo_esep

RAW_COORD(eo_esep, EO_ESEP_MAX);
RAW_COORD(partial_eo_esep, 0);

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
    \
    static struct coord coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = SYM_CLASSES * RAW_MAX,\
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

static long long get_sym_comp(cube_t x, struct coord *c)
{
    long long r = c->sym->get(x);
    x = apply_sym(x, c->sym->info[r].sym);
    r = c->sym->info[r].class;
    return r * c->raw->max + c->raw->get(x);
}

static cube_t set_sym_comp(long long r, struct coord *c)
{
    cube_t x = c->sym->set(c->sym->to_rep[r/c->raw->max]);
    cube_t y = c->raw->set(r%c->raw->max);
    return compose(x, y);
}

COORD(phase1, co_csep, 3393, partial_eo_esep, 0);
COORD(phase1_full, co_csep, 3393, eo_esep, EO_ESEP_MAX);

////////////////////////////////////////////////////////////////////////////////

static int is_self_sym(struct coord *c, cube_t x, int s)
{
    return c->sym->self_syms[c->sym->get(x)] >> s & 1;
}

static void init_coord(struct coord *c)
{
    long long table_max = PRUNE_EXT_62(c->max);
    FILE *fp;
    char filename[256];
    snprintf(filename, sizeof(filename), "e%d.bin", EO_VARIANT);
    c->sym->to_rep = malloc(sizeof(int)*c->sym->classes);
    c->sym->info = malloc(4*c->sym->max);
    c->table = table_new(table_max, 2);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(c->sym->to_rep, sizeof(int)*c->sym->classes, 1, fp);
        fread(c->sym->info, 4*c->sym->max, 1, fp);
        fread(c->table, TABLE_SIZE(table_max, 2), 1, fp);
        LOG("read '%s'\n", filename);
    }
    else
        if ((fp = fopen(filename, "wb")))
    {
        init_sym(c->sym);
        init_prune_table(c);
        fwrite(c->sym->to_rep, sizeof(int)*c->sym->classes, 1, fp);
        fwrite(c->sym->info, 4*c->sym->max, 1, fp);
        fwrite(c->table, TABLE_SIZE(table_max, 2), 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}
