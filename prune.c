static void clear_stderr(void)
{
    fprintf(stderr, "\r");
    for (int i=0; i<80; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\r");
}

static void fill_sym_table(struct sym_coord *c)
{
    for (long long i=0; i<c->max; ++i)
    {
        cube_t x = c->set(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            long long k = c->get(apply_sym(x, s));
            c->self_syms[i] |= (long long)(i==k)<<s%48;
        }
        if (i%(c->max/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/c->max);
    }
    clear_stderr();

    int class = 0;
    memset(c->info, 0xff, 4*c->max);
    for (long long i=0; i<c->max; ++i)
    {
        if (*(unsigned *)&c->info[i] != -1u)
            continue;
        cube_t x = c->set(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            int k = c->get(apply_sym(x, s));
            if (s && k==i)
                continue;
            c->info[k].class = class;
            c->info[k].sym = inv_sym[s];
        }
        c->to_rep[class++] = i;
        if (i%(c->max/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/c->max);
    }
    clear_stderr();
    LOG("%s classes : %d\n", c->name, class);
    ASSERT(class == c->classes);
}

static struct map *fill_prune_map(void)
{
    struct coord *c = &coord_phase1_full;
    struct map *map = map_new();
    map_set(map, c->get(new_cube()), 0);
    for (int depth=1; depth<=MAP_DEPTH; ++depth)
        for (long long i=0; i<MAP_CAPACITY; ++i)
        {
            if (map->data[i].val == depth-1)
                for (int m=0; m<18; ++m)
                {
                    cube_t x = apply_move(c->set(map->data[i].key), m);
                    for (int s=0; s<NUM_SYMS; ++s)
                    {
                        if (!is_self_sym(c, x, s))
                            continue;
                        long long k = c->get(apply_sym(x, s));
                        if (map_get(map, k) == MAP_VAL_MAX)
                            map_set(map, k, depth);
                    }
                }
            if ((i+MAP_CAPACITY*(depth-1))%(MAP_DEPTH*MAP_CAPACITY/1000)==0)
                fprintf(stderr, "\rdepth=%d load=%.1f%% mem=%.1fMB",
                        depth, 100.0*map->count/MAP_CAPACITY, 8.0*map->count/1e6);
        }
    clear_stderr();
    return map;
}

static int fill_prune_table_dfs(void *varg)
{
    struct fill_prune_table_arg *arg = varg;

    void dfs(cube_t x, int start_depth)
    {
        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            int class = arg->c->get(x)%arg->c->sym->classes;
            mtx_lock(&arg->mutexes[class]);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                if (!is_self_sym(arg->c, x, s))
                    continue;
                int v = MAX(depth-PRUNE_BASE, 0);
                long long i = arg->c->get(apply_sym(x, s));
                long long j = PRUNE_EXT_62(i);
                long long k = PRUNE_MIN_62(i);
                TABLE_SET_MIN(arg->c->table, 2, j, v);
                TABLE_SET_MIN(arg->c->table, 4, k/2, depth);
            }
            mtx_unlock(&arg->mutexes[class]);
            if (depth >= MAP_DEPTH &&
                depth < arg->depth &&
                depth <= map_get(arg->map, coord_phase1_full.get(x)))
                *top++ = (struct search_node){x, move, depth};
        }

        push(x, EMPTY_MOVE, start_depth);
        while (top>stack)
        {
            struct search_node cur = *--top;
            FOREACH_MOVE(cur.move)
                push(apply_move(cur.cube, m), m, cur.depth+1);
        }
    }

    int start = arg->thread_id*MAP_CAPACITY/THREADS;
    int end = arg->thread_id==THREADS-1 ? MAP_CAPACITY : (arg->thread_id+1)*MAP_CAPACITY/THREADS;
    for (int i=start; i<end; ++i)
    {
        if (arg->map->data[i].val <= MAP_DEPTH)
            dfs(coord_phase1_full.set(arg->map->data[i].key), arg->map->data[i].val);
        if (arg->thread_id==0 && i%(end/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/end);
    }
    return 0;
}

static void fill_prune_table(struct coord *c)
{
    struct map *map = fill_prune_map();
    mtx_t mutexes[c->sym->classes];
    for (int i=0; i<c->sym->classes; ++i)
        mtx_init(&mutexes[i], mtx_plain);
    struct fill_prune_table_arg args[THREADS];
    thrd_t threads[THREADS];
    for (int i=0; i<THREADS; ++i)
    {
        args[i].mutexes = mutexes;
        args[i].thread_id = i;
        args[i].c = c;
        args[i].depth = PRUNE_BASE+2;
        args[i].map = map;
        thrd_create(&threads[i], fill_prune_table_dfs, &args[i]);
    }
    for (int i=0; i<THREADS; ++i)
        thrd_join(threads[i], NULL);
    for (int i=0; i<c->sym->classes; ++i)
        mtx_destroy(&mutexes[i]);
    clear_stderr();
    if (!VERBOSE)
        return;
    long long dist[4] = {0};
    for (int i=0; i<c->max; ++i)
        dist[table_get(c->table, 2, i)]++;
    for (int i=PRUNE_BASE; i<PRUNE_BASE+4; ++i)
        LOG("%s[%d] = %lld\n", i<10?" ":"", i, dist[i-PRUNE_BASE]);
}
