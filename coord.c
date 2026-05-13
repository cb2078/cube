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
    cube_t x = new_cube();
    set_csep(&x, r % CSEP_MAX);
    set_co(&x, r / CSEP_MAX);
    return x;
}

SYM_COORD(co_csep, CO_CSEP_MAX, CO_CSEP_CLASSES);

////////////////////////////////////////////////////////////////////////////////

#define COORD(NAME, SYM, SYM_CLASSES, RAW, RAW_MAX)\
    \
    static struct coord coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = SYM_CLASSES * RAW_MAX,\
        .sym = &sym_coord_##SYM,\
    };\

static long long get_phase1(cube_t x)
{
    long long r = get_co_csep(x);
    x = apply_sym(x, coord_phase1.sym->info[r].sym);
    r = coord_phase1.sym->info[r].class;
    return r * PARTIAL_EO_ESEP_MAX + (get_eo(x) & (PARTIAL_EO_MAX-1)) * ESEP_MAX + get_esep(x);
}

static cube_t set_phase1(long long r)
{
    cube_t x = set_co_csep(coord_phase1.sym->to_rep[r / PARTIAL_EO_ESEP_MAX]);
    set_esep(&x, r % ESEP_MAX);
    set_eo(&x, r % PARTIAL_EO_ESEP_MAX / ESEP_MAX);
    return x;
}

static long long get_phase1_full(cube_t x)
{
    long long r = get_co_csep(x);
    x = apply_sym(x, coord_phase1.sym->info[r].sym);
    r = coord_phase1.sym->info[r].class;
    return r * EO_ESEP_MAX + get_eo(x) * ESEP_MAX + get_esep(x);
}

static cube_t set_phase1_full(long long r)
{
    cube_t x = set_co_csep(coord_phase1.sym->to_rep[r / EO_ESEP_MAX]);
    set_esep(&x, r % ESEP_MAX);
    set_eo(&x, r % EO_ESEP_MAX / ESEP_MAX);
    return x;
}

COORD(phase1, co_csep, CO_CSEP_CLASSES, partial_eo_esep, 0);
COORD(phase1_full, co_csep, CO_CSEP_CLASSES, eo_esep, EO_ESEP_MAX);

////////////////////////////////////////////////////////////////////////////////

static int is_self_sym(struct coord *c, cube_t x, int s)
{
    return c->sym->self_syms[c->sym->get(x)] >> s & 1;
}

static void init_sym(struct sym_coord *c)
{
    FILE *fp;
    char filename[256];
    sprintf(filename, "%s.bin", c->name);
    c->to_rep = malloc(sizeof(int)*c->classes);
    c->info = malloc(4*c->max);
    c->self_syms = malloc(sizeof(long long)*c->max);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(c->to_rep, sizeof(int)*c->classes, 1, fp);
        fread(c->info, 4*c->max, 1, fp);
        fread(c->self_syms, sizeof(long long)*c->max, 1, fp);
        LOG("read '%s'\n", filename);
    }
    else if ((fp = fopen(filename, "wb")))
    {
        fill_sym_table(c);
        fwrite(c->to_rep, sizeof(int)*c->classes, 1, fp);
        fwrite(c->info, 4*c->max, 1, fp);
        fwrite(c->self_syms, sizeof(long long)*c->max, 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}

static void init_coord(struct coord *c)
{
    init_sym(c->sym);

    long long table_max = PRUNE_EXT_62(c->max);
    FILE *fp;
    char filename[256];
    sprintf(filename, "e%d.bin", EO_VARIANT);
    c->table = table_new(table_max, 2);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(c->table, TABLE_SIZE(table_max, 2), 1, fp);
        LOG("read '%s'\n", filename);
    }
    else if ((fp = fopen(filename, "wb")))
    {
        fill_prune_table(c);
        fwrite(c->table, TABLE_SIZE(table_max, 2), 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}
