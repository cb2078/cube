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

static void init_sym(struct sym_coord *c)
{
    ASSERT(!c->self_syms);
    c->self_syms = malloc(sizeof(long long)*c->max);
    for (long long i=0; i<c->max; ++i)
    {
        cube_t x = c->set(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = c->get(y);
            c->self_syms[i] |= (long long)(i==k)<<s;
        }
        print_completion(i, c->max);
    }
    clear_stderr();

    int class = 0;
    memset(c->to_class, 0xff, sizeof(c->to_class[0])*c->max);
    for (long long i=0; i<c->max; ++i)
    {
        if (c->to_class[i] != -1)
            continue;
        cube_t x = c->set(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = c->get(y);
            c->to_class[k] = class;
            c->to_sym[k] = inv_sym[s];
        }
        c->to_rep[class++] = i;
        print_completion(i, c->max);
    }
    clear_stderr();
    LOG("%s classes : %d\n", c->name, class);
    ASSERT(class == c->classes);
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
                int class = j/arg->c->raw->max;
                mtx_lock(&arg->mutexes[class]);
                if (!arg->backsearch && table_get(arg->c->table, j) == arg->c->table->mask)
                {
                    table_set(arg->c->table, j, arg->depth);
                    for (int s=1; s<NUM_SYMS; ++s)
                    {
                        if (~arg->c->sym->self_syms[arg->c->sym->get(x)]>>s&1)
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

static int init_prune_map(struct coord *c, int max_depth, struct map *map)
{
    int t = map->count*1000/MAP_CAPACITY;
    for (long long i=0; i<MAP_CAPACITY; ++i)
    {
        if (map->data[i].val == max_depth-1)
        {
            for (int m=0; m<18; ++m)
            {
                cube_t x = apply_move(c->set(map->data[i].key), m);
                long long j = c->get(x);
                if (map_get(map, j) == MAP_VAL_MAX)
                    for (int s=0; s<NUM_SYMS; ++s)
                    {
                        if (~c->sym->self_syms[c->sym->get(x)]>>s&1)
                            continue;
                        cube_t y = apply_sym(x, s);
                        long long k = c->get(y);
                        if (map_get(map, k) == MAP_VAL_MAX)
                            map_set(map, k, max_depth);
                    }
            }
        }
        if (map->count*1000/MAP_CAPACITY > t)
        {
            fprintf(stderr, "\rdepth=%d load=%.1f%% mem=%.1fMB",
                    max_depth, (double)map->count/MAP_CAPACITY*100, (double)map->count*8/1e6);
            t++;
        }
    }
    return 0;
}

// NOTE consider merging these
struct init_prune_table_dfs_forward_arg
{
    mtx_t *mutexes;
    int thread_id;
    int t;
    struct coord *c;
    int depth;
    long long start;
    long long end;
    struct map *map;
    int start_depth;
};

static int init_prune_table_dfs_forward(void *varg)
{
    struct init_prune_table_dfs_forward_arg *arg = varg;

    void dfs(cube_t x)
    {
        // TODO move this into a header
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
            if (depth > arg->depth)
                return;
            if (depth > map_get(arg->map, coord_eo_full.get(x)))
                return;
            // TODO move this to a function
            int class = arg->c->get(x)/arg->c->raw->max;
            mtx_lock(&arg->mutexes[class]);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                if (~arg->c->sym->self_syms[arg->c->sym->get(x)]>>s&1)
                    continue;
                long long k = arg->c->get(apply_sym(x, s));
                if (table_get(arg->c->table, k) == arg->c->table->mask)
                    table_set(arg->c->table, k, depth);
            }
            mtx_unlock(&arg->mutexes[class]);
            *top++ = (struct search_node){x, move, depth};
        }

        push(x, EMPTY_MOVE, arg->start_depth);
        while (top>stack)
        {
            struct search_node cur = *--top;
            FOREACH_MOVE(cur.move)
                push(apply_move(cur.cube, m), m, cur.depth+1);
        }
        if (!arg->thread_id && arg->c->table->count*10000/arg->c->max>arg->t)
        {
            print_completion(arg->c->table->count, arg->c->max);
            fprintf(stderr, " depth=%d", arg->depth);
            arg->t++;
        }
    }

    for (int i=arg->start; i<arg->end; ++i)
        if (arg->map->data[i].val == arg->start_depth)
            dfs(coord_eo_full.set(arg->map->data[i].key));
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
    int map_depth = 9;
    struct map *map = 0;
    if (EO_PARTIAL)
    {
        map = map_new();
        map_set(map, c->get(new_cube()), 0);
        for (int depth=1; map->count<coord_eo_full.max && depth < map_depth; ++depth)
        {
            long long m = map->count;
            init_prune_map(&coord_eo_full, depth, map);
            clear_stderr();
        }
    }

    thrd_t threads[THREADS];
    mtx_t mutexes[c->sym->classes];
    for (int i=0; i<c->sym->classes; ++i)
        mtx_init(&mutexes[i], mtx_plain);
    // TODO do this in `table_new`
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    for (int depth=1; c->table->count<c->max && depth<c->table->mask; ++depth)
    {
        int backsearch = c->table->count>c->max/2;
        long long m = c->table->count;
        if (!EO_PARTIAL)
        {
            struct init_prune_table_bfs_arg args[THREADS];
            for (int i=0; i<THREADS; ++i)
            {
                args[i].mutexes = mutexes;
                args[i].thread_id = i;
                args[i].t = 0;
                args[i].c = c;
                args[i].depth = depth;
                args[i].backsearch = backsearch;
                args[i].start = i*c->max/THREADS;
                args[i].end = i==THREADS-1 ? c->max : (i+1)*c->max/THREADS;
                thrd_create(&threads[i], init_prune_table_bfs, &args[i]);
            }
            for (int i=0; i<THREADS; ++i)
                thrd_join(threads[i], NULL);
        }
        // TODO use the map for the full coordinates as well (this is
        // definitely faster for full EO)
        else if (depth < map_depth)
        {
            for (int i=0; i<MAP_CAPACITY; ++i)
                if (map->data[i].val == depth)
                {
                    long long j = c->get(coord_eo_full.set(map->data[i].key));
                    if (table_get(c->table, j) == c->table->mask)
                        table_set(c->table, j, depth);
                }
        }
        else if (backsearch)
        {
            init_prune_table_dfs_backward(c, depth);
        }
        else
        {
            struct init_prune_table_dfs_forward_arg args[THREADS];
            for (int i=0; i<THREADS; ++i)
            {
                args[i].mutexes = mutexes;
                args[i].thread_id = i;
                args[i].t = c->table->count*10000/c->max;
                args[i].c = c;
                args[i].depth = depth;
                args[i].start = i*MAP_CAPACITY/THREADS;
                args[i].end = i==THREADS-1 ? MAP_CAPACITY : (i+1)*MAP_CAPACITY/THREADS;
                args[i].map = map;
                args[i].start_depth = map_depth-1;
                thrd_create(&threads[i], init_prune_table_dfs_forward, &args[i]);
            }
            for (int i=0; i<THREADS; ++i)
                thrd_join(threads[i], NULL);
        }
        clear_stderr();
        LOG("%s[%d] = %lld\n", depth<10?" ":"", depth, c->table->count-m);
    }
    if (c->table->count!=c->max)
        LOG("skpped %lld entries\n", c->max-c->table->count);
    ASSERT(c->table->count==c->max);
    for (int i=0; i<c->sym->classes; ++i)
        mtx_destroy(&mutexes[i]);
}
