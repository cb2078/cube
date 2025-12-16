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
    ASSERT(!c->self_syms);
    c->self_syms = malloc(sizeof(long long)*c->sym.max);
    for (long long i=0; i<c->sym.max; ++i)
    {
        cube_t x = c->sym.set(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = c->sym.get(y);
            c->self_syms[i] |= (long long)(i==k)<<s;
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
        for (int s=0; s<NUM_SYMS; ++s)
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
    LOG("%s classes : %d\n", c->filename, class);
    ASSERT(class == c->sym.classes);
}

struct init_prune_table_bfs_arg
{
    mtx_t *mutexes;
    int thread_id;
    int t;
    struct coord *c;
    int depth;
    int backsearch;
    long long start;
    long long end;
};

static int init_prune_table_bfs(void *varg)
{
    struct init_prune_table_bfs_arg *arg = varg;
    for (long long i=arg->start; i<arg->end; ++i)
    {
        if (table_get_atomic(arg->c->table, i) == (arg->backsearch ? arg->c->table->mask : arg->depth-1))
        {
            for (int m=0; m<18; ++m)
            {
                cube_t x = apply_move(arg->c->set(i), m);
                long long j = arg->c->get(x);
                if (arg->backsearch && table_get_atomic(arg->c->table, j) == arg->depth-1)
                {
                    table_set_atomic(arg->c->table, i, arg->depth);
                    break;
                }
                int class = j/arg->c->raw.max;
                mtx_lock(&arg->mutexes[class]);
                if (!arg->backsearch && table_get(arg->c->table, j) == arg->c->table->mask)
                {
                    table_set(arg->c->table, j, arg->depth);
                    for (int s=1; s<NUM_SYMS; ++s)
                    {
                        if (~arg->c->self_syms[arg->c->sym.get(x)]>>s&1)
                            continue;
                        cube_t y = apply_sym(x, s);
                        long long k = arg->c->get(y);
                        if (table_get(arg->c->table, k) == arg->c->table->mask)
                            table_set(arg->c->table, k, arg->depth);
                    }
                }
                mtx_unlock(&arg->mutexes[class]);
            }
        }
        if (!arg->thread_id && arg->c->table->count*10000/arg->c->max>arg->t)
        {
            print_completion(arg->c->table->count, arg->c->max);
            fprintf(stderr, " depth=%d%s", arg->depth, arg->backsearch?" (backsearch)":"");
            // fprintf(stderr, " t=%d", arg->t);
            arg->t++;
        }
    }
    return 0;
}

static void init_prune_table_parallel(struct coord *c, int depth, int backsearch)
{
    mtx_t mutexes[c->sym.classes];
    for (int i=0; i<c->sym.classes; ++i)
    {
        mtx_init(&mutexes[i], mtx_plain);
    }
    thrd_t threads[THREADS];
    struct init_prune_table_bfs_arg args[THREADS];
    for (int i=0; i<THREADS; ++i)
    {
        args[i].mutexes = mutexes;
        args[i].thread_id = i;
        args[i].t = 0;
        args[i].c = c;
        args[i].depth = depth;
        args[i].backsearch = backsearch;
        // TODO would it be faster to for threads to start at 0, 1, 2, ... and
        // increment by the number of threads?
        args[i].start = i*c->max/THREADS;
        args[i].end = i==THREADS-1 ? c->max : (i+1)*c->max/THREADS;
        thrd_create(&threads[i], init_prune_table_bfs, &args[i]);
    }
    for (int i=0; i<THREADS; ++i)
    {
        thrd_join(threads[i], NULL);
    }
    for (int i=0; i<c->sym.classes; ++i)
    {
        mtx_destroy(&mutexes[i]);
    }
}

// NOTE this does not give the distance for very few cubes (for partial eo
// coordinates). I believe this is because the full EO coordinate may be
// different in the current cube and the one where the pruning value is stored.
//
// Since this affects such a small amount of nodes, it doesn't make much of a
// difference when solving.
static int init_prune_table_dfs_forward(struct coord *c, int max_depth)
{
    struct search_node
    {
        cube_t cube;
        int move;
        int depth;
    };

    struct search_node stack[256];
    struct search_node *top = stack;

    void push(cube_t x, int move, int depth)
    {
        if (depth>max_depth)
            return;
        if (table_get(c->table, c->get(x)) < depth)
            return;
        if (table_get(c->table, c->get(x)) == c->table->mask)
            table_set(c->table, c->get(x), depth);
        *top++ = (struct search_node){x, move, depth};
    }

    push(new_cube(), EMPTY_MOVE, 0);
    while (top>stack)
    {
        struct search_node cur = *--top;
        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), m, cur.depth+1);
    }
    return 0;
}

static int init_prune_table_dfs_backward(struct coord *c, int max_depth)
{
    int h(cube_t x)
    {
        int r = table_get(c->table, c->get(x));
        return r == c->table->mask ? max_depth : r ?: get_eo(x)>0;
    }

    int dlA(cube_t x)
    {
        struct search_node
        {
            cube_t cube;
            int move;
            int depth;
        };

        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            if (h(x)+depth <= max_depth)
                *top++ = (struct search_node){x, move, depth};
        }

        push(x, EMPTY_MOVE, 0);
        while (top>stack)
        {
            struct search_node cur = *--top;
            if (!h(cur.cube))
                return 0;
            FOREACH_MOVE(cur.move)
                push(apply_move(cur.cube, m), m, cur.depth+1);
        }
        return 1;
    }

    for (long long i=0; i<c->max; i++)
        for (int j=0; j<EO_MAX; j+=PARTIAL_EO_MAX)
        {
            if (table_get(c->table, i) != c->table->mask)
                break;
            if (!j && i%(c->max/100)==0)
                printf("%lld%%\n", i/(c->max/100));
            if (!dlA(compose(c->set(i), set_eo(j))))
                table_set(c->table, i, max_depth);
        }
    return 0;
}

static void init_prune_table(struct coord *c)
{
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    for (int depth=1; c->table->count<c->max && depth<c->table->mask; ++depth)
    {
        int backsearch = c->table->count>c->max/2;
        long long m = c->table->count;
        if (EO_VARIANT == 0 || EO_VARIANT == 11)
        {
            init_prune_table_parallel(c, depth, backsearch);
        }
        else
        {
            if (backsearch)
                init_prune_table_dfs_backward(c, depth);
            else
                init_prune_table_dfs_forward(c, depth);
        }
        clear_stderr();
        LOG("%s[%d] = %lld\n", depth<10?" ":"", depth, c->table->count-m);
    }
    if (c->table->count!=c->max)
        LOG("skpped %lld entries\n", c->max-c->table->count);
    ASSERT(c->table->count==c->max);
}

// TODO move to coord.c
// merge this in the commit
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
