#define SYM_COORD(NAME, MAX, CLASSES)\
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

#define get_orbit get_orbit_fast

static cube_t set_orbit(long long r)
{
    cube_t x = new_cube();
    set_orbit_fast(&x, r);
    return x;
}

SYM_COORD(co_csep, CO_CSEP_MAX, CO_CSEP_CLASSES);
SYM_COORD(orbit, ORBIT_MAX, ORBIT_CLASSES);

////////////////////////////////////////////////////////////////////////////////

#define COORD(NAME, SYM, SYM_CLASSES, RAW, RAW_MAX, BITS)\
    static struct coord coord_##NAME =\
    {\
        .name = #NAME,\
        .get = get_##NAME,\
        .set = set_##NAME,\
        .max = SYM_CLASSES * RAW_MAX,\
        .sym = &sym_coord_##SYM,\
        .bits = BITS,\
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

static long long get_phase2(cube_t x)
{
    long long r = get_orbit(x);
    x = apply_sym(x, coord_phase2.sym->info[r].sym);
    r = coord_phase2.sym->info[r].class;
    return r * EO_MAX + get_eo(x);
}

static cube_t set_phase2(long long r)
{
    cube_t x = set_orbit(coord_phase2.sym->to_rep[r / EO_MAX]);
    set_eo(&x, r % EO_MAX);
    return x;
}

static long long get_phase2_full(cube_t x)
{
    long long r = get_co_csep(x);
    x = apply_sym(x, coord_phase1.sym->info[r].sym);
    return get_phase1_full(x) * ORBIT_MAX + get_orbit_slow(x);
}

static cube_t set_phase2_full(long long r)
{
    cube_t x = set_phase1_full(r / ORBIT_MAX);
    cube_t y = new_cube();
    set_orbit_slow(&y, r % ORBIT_MAX);
    return compose(y, x);
}

COORD(phase1, co_csep, CO_CSEP_CLASSES, partial_eo_esep, 0, 2);
COORD(phase1_full, co_csep, CO_CSEP_CLASSES, eo_esep, EO_ESEP_MAX, 2);
COORD(phase2, orbit, ORBIT_CLASSES, eo, EO_MAX, 4);
COORD(phase2_full, co_csep, CO_CSEP_CLASSES, eo_esep_orbit, EO_ESEP_MAX*ORBIT_MAX, 4);

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

static void init_coord(struct coord *c, void (*fill_prune_table)(void))
{
    init_sym(c->sym);

    long long table_max = c->bits==2 ? PRUNE_EXT_62(c->max) : c->max;
    FILE *fp;
    char filename[256];
    snprintf(filename, sizeof(filename), c==&coord_phase1?"%se%d.bin":"%s.bin", c->name, EO_VARIANT);
    c->table = table_new(table_max, c->bits);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(c->table, TABLE_SIZE(table_max, c->bits), 1, fp);
        LOG("read '%s'\n", filename);
    }
    else if ((fp = fopen(filename, "wb")))
    {
        fill_prune_table();
        fwrite(c->table, TABLE_SIZE(table_max, c->bits), 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}
