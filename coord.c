static inline long long get_sym_coord(cube_t x)
{
    return get_co(x) * CSEP_MAX + get_csep(x);
}

static inline cube_t set_sym_coord(long long r)
{
    cube_t x = new_cube();
    set_csep(&x, r % CSEP_MAX);
    set_co(&x, r / CSEP_MAX);
    return x;
}

static inline long long get_coord(cube_t x, int i)
{
    long long r = get_sym_coord(x);
    x = apply_sym(x, sym_coord.info[r].sym);
    r = sym_coord.info[r].class;
    return r * (ESEP_MAX<<i) + (get_eo(x) & (1<<i)-1) * ESEP_MAX + get_esep(x);
}

static inline cube_t set_coord(long long r, int i)
{
    cube_t x = set_sym_coord(sym_coord.to_rep[r / (ESEP_MAX<<i)]);
    set_esep(&x, r % ESEP_MAX);
    set_eo(&x, r / ESEP_MAX & (1<<i)-1);
    return x;
}

////////////////////////////////////////////////////////////////////////////////

static void init_sym(void)
{
    FILE *fp;
    char filename[256];
    sprintf(filename, "%s.bin", SYM_COORD_NAME);
    sym_coord.to_rep = malloc(sizeof(int)*SYM_COORD_CLASSES);
    sym_coord.info = malloc(INFO_BITS/8*SYM_COORD_MAX);
    sym_coord.self_syms = malloc(sizeof(long long)*SYM_COORD_MAX);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(sym_coord.to_rep, sizeof(int)*SYM_COORD_CLASSES, 1, fp);
        fread(sym_coord.info, 4*SYM_COORD_MAX, 1, fp);
        fread(sym_coord.self_syms, sizeof(long long)*SYM_COORD_MAX, 1, fp);
        LOG("read '%s'\n", filename);
    }
    else if ((fp = fopen(filename, "wb")))
    {
        fill_sym_table();
        fwrite(sym_coord.to_rep, sizeof(int)*SYM_COORD_CLASSES, 1, fp);
        fwrite(sym_coord.info, 4*SYM_COORD_MAX, 1, fp);
        fwrite(sym_coord.self_syms, sizeof(long long)*SYM_COORD_MAX, 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}

static void init_coord(void)
{
    long long table_max = PRUNE_EXT_62(COORD_MAX);
    FILE *fp;
    char filename[256];
    snprintf(filename, sizeof(filename), "%se%d.bin", COORD_NAME, EO_VARIANT);
    coord.bits = 2;
    coord.table = table_new(table_max, coord.bits);
    if (!NO_INPUT && (fp = fopen(filename, "rb")))
    {
        fread(coord.table, TABLE_SIZE(table_max, coord.bits), 1, fp);
        LOG("read '%s'\n", filename);
    }
    else if ((fp = fopen(filename, "wb")))
    {
        fill_prune_table();
        fwrite(coord.table, TABLE_SIZE(table_max, coord.bits), 1, fp);
        LOG("wrote '%s'\n", filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", filename);
    }
    fclose(fp);
}
