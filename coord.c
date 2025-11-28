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
        .num_syms = 16,\
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

static long long get_flip_ud_slice(cube_t x)
{
    long long r = get_eo(x);
    for (int i=0; i<NUM_EDGES; ++i)
        x.edges[i] &= 0x0f;
    return r * choose[12][4] + get_combination(x.edges, 12, 4);
}

static cube_t set_flip_ud_slice(long long r)
{
    cube_t x = new_cube();
    set_combination(x.edges, 12, 4, r%choose[12][4]);
    set_eo(&x, r/choose[12][4]);
    return x;
}

static long long get_twist(cube_t x)
{
    return get_co(x);
}

static cube_t set_twist(long long r)
{
    cube_t x = new_cube();
    set_co(&x, r);
    return x;
}

COORD(phase1, 0,
      flip_ud_slice, choose[12][4]*pow2[NUM_EDGES-1], 64430,
      twist, pow3[NUM_CORNERS-1]);

////////////////////////////////////////////////////////////////////////////////

static long long get_corner_perm(cube_t x)
{
    return get_permutation(x.corners, 8);
}

static cube_t set_corner_perm(long long r)
{
    cube_t x = new_cube();
    set_permutation(x.corners, 8, r);
    return x;
}

static long long get_ud_edge_perm(cube_t x)
{
    for (int i=4; i<NUM_EDGES; ++i)
        x.edges[i] -= 4;
    return get_permutation(x.edges+4, 8);
}

static cube_t set_ud_edge_perm(long long r)
{
    cube_t x = new_cube();
    set_permutation(x.edges+4, 8, r);
    for (int i=4; i<NUM_EDGES; ++i)
        x.edges[i] += 4;
    return x;
}

COORD(phase2, DR_MASK,
      corner_perm, fact[8], 2768,
      ud_edge_perm, fact[8]);


////////////////////////////////////////////////////////////////////////////////

#define TWIST_MAX pow3[NUM_CORNERS-1]

static long long get_ud_slice_sorted(cube_t x)
{
    for (int i=0; i<NUM_EDGES; ++i) x.edges[i] &= 0x0f;
    return get_partial_permutation(x.edges, 12, 4);
}

static cube_t set_ud_slice_sorted(long long r)
{
    cube_t x = new_cube();
    set_partial_permutation(x.edges, 12, 4, r);
    return x;
}

static long long get_orientation(cube_t x)
{
    return get_eo(x) * TWIST_MAX + get_twist(x);
}

static cube_t set_orientation(long long r)
{
    cube_t x = new_cube();
    set_eo(&x, r/TWIST_MAX);
    set_co(&x, r%TWIST_MAX);
    return x;
}

COORD(optimal, 0,
      ud_slice_sorted, pick[12][4], 788,
      orientation, pow2[NUM_EDGES-1]*pow3[NUM_CORNERS-1]);

////////////////////////////////////////////////////////////////////////////////


static void print_completion(long long i, long long n)
{
    fprintf(stderr, "\rcompletion=%.2f", (double)i/n*100);
}

static void clear_stderr(void)
{
    fprintf(stderr, "\r");
    for (int i=0; i<80; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\r");
}

static void init_sym(struct coord *c)
{
    if (!c->num_syms)
        return;

    ASSERT(!c->self_syms);
    c->self_syms = malloc(sizeof(int)*c->sym.max);
    for (long long i=0; i<c->sym.max; ++i)
    {
        cube_t x = c->sym.set(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = c->sym.get(y);
            c->self_syms[i] |= (i==k)<<s;
        }
        print_completion(i, c->sym.max);
    }
    clear_stderr();

    int class = 0;
    memset(c->to_class, 0xff, sizeof(c->to_class[0])*c->sym.max);
    for (long long i=0; i<c->sym.max; ++i)
    {
        if (c->to_class[i] != -1)
            continue;
        cube_t x = c->sym.set(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = c->sym.get(y);
            c->to_class[k] = class;
            c->to_sym[k] = inv_sym[s];
        }
        c->to_rep[class++] = i;
        print_completion(i, c->sym.max);
    }
    clear_stderr();
    ASSERT(class == c->sym.classes);
}

struct init_prune_table_arg
{
    struct coord *c;
    int depth;
    int backsearch;
    long long start;
    long long end;
};

// NOTE using atomics here is faster than a mutex up to four threads, however
// this should be tested with a larger number of threads
static int init_prune_table_thread(void *__arg)
{
    struct init_prune_table_arg *arg = __arg;
    for (long long i=arg->start; i<arg->end; ++i)
        if (table_get(arg->c->table, i) == (arg->backsearch ? arg->c->table->mask : arg->depth-1))
        {
            int moves[18], length;
            possible_moves(moves, &length, 0xff, arg->c->move_mask);
            for (int j=0; j<length; ++j)
            {
                cube_t x = apply_move(arg->c->set(i), moves[j]);
                long long k = arg->c->get(x);
                if (!arg->backsearch && table_get_atomic(arg->c->table, k) == arg->c->table->mask)
                {
                    table_set_atomic(arg->c->table, k, arg->depth);
                    if (!arg->c->num_syms)
                        continue;
                    for (int s=1; s<arg->c->num_syms; ++s)
                    {
                        if (~arg->c->self_syms[arg->c->sym.get(x)]>>s&1)
                            continue;
                        cube_t y = apply_sym(x, s);
                        long long l = arg->c->get(y);
                        if (table_get_atomic(arg->c->table, l) == arg->c->table->mask)
                            table_set_atomic(arg->c->table, l, arg->depth);
                    }
                }
                else if (arg->backsearch && table_get_atomic(arg->c->table, k) == arg->depth-1)
                {
                    table_set_atomic(arg->c->table, i, arg->depth);
                    break;
                }
            }
        }
    return 0;
}

static void init_prune_table_parallel(struct coord *c, int depth, int backsearch)
{
    thrd_t threads[THREADS];
    struct init_prune_table_arg args[THREADS] = {0};
    for (int i=0; i<THREADS; ++i)
    {
        args[i].c = c;
        args[i].depth = depth;
        args[i].backsearch = backsearch;
        args[i].start = i*c->max/THREADS;
        args[i].end = i==THREADS-1 ? c->max : (i+1)*c->max/THREADS;
        thrd_create(&threads[i], init_prune_table_thread, &args[i]);
    }
    for (int i=0; i<THREADS; ++i)
    {
        thrd_join(threads[i], NULL);
    }
}

static void init_prune_table(struct coord *c)
{
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    for (int depth=1, t=0, backsearch=0; c->table->count<c->max && depth<c->table->mask; ++depth)
    {
        long long m = c->table->count;
        init_prune_table_parallel(c, depth, backsearch);
        backsearch = c->table->count>c->max/2;
        LOG("%s[%d] = %lld\n", depth<10?" ":"", depth, c->table->count-m);
    }
    if (c->table->count!=c->max)
        LOG("skpped %lld entries\n", c->max-c->table->count);
}

static void init_coord(struct coord *c)
{
    FILE *fp;
    c->to_rep   = malloc(sizeof(int)*c->sym.classes);
    c->to_class = malloc(sizeof(int)*c->sym.max);
    c->to_sym   = malloc(sizeof(int)*c->sym.max);
    c->table    = table_new(c->max, 4, c->filename);

    if ((fp = fopen(c->filename, "rb")))
    {
        fread(c->to_rep,      sizeof(int)*c->sym.classes, 1, fp);
        fread(c->to_class,    sizeof(int)*c->sym.max,     1, fp);
        fread(c->to_sym,      sizeof(int)*c->sym.max,     1, fp);
        fread(c->table->data, c->table->size,             1, fp);
        LOG("read '%s'\n", c->filename);
    }
    else if ((fp = fopen(c->filename, "wb")))
    {
        init_sym(c);
        init_prune_table(c);
        fwrite(c->to_rep,      sizeof(int)*c->sym.classes, 1, fp);
        fwrite(c->to_class,    sizeof(int)*c->sym.max,     1, fp);
        fwrite(c->to_sym,      sizeof(int)*c->sym.max,     1, fp);
        fwrite(c->table->data, c->table->size,             1, fp);
        LOG("wrote '%s'\n", c->filename);
    }
    else
    {
        ERROR("couldn't write '%s'\n", c->filename);
    }
    fclose(fp);
}
