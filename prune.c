static void clear_stderr(void)
{
    fprintf(stderr, "\r");
    for (int i=0; i<80; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\r");
}

static void log_dist(struct coord *c, int bits, int base, int max_depth)
{
    if (!VERBOSE)
        return;
    long long dist[21] = {0}, sum = 0;
    for (int i=0; i<c->max; ++i)
        dist[table_get(c->table, bits, i)]++;
    for (int i=base; i<max_depth; ++i)
    {
        LOG("%s[%d] = %lld", i<10?" ":"", i, dist[i-base]);
        if (i>base)
            LOG("\t(%.1fx)\n", (float)dist[i-base]/dist[i-1-base]);
        else
            LOG("\n");
        sum+=dist[i-base];
    }
    LOG("skipped %lld entries (%.2f%% filled)\n", c->max-sum, 100.0*(sum)/c->max);
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

    int start = arg->thread_id*MAP_CAPACITY/WORKERS;
    int end = arg->thread_id==WORKERS-1 ? MAP_CAPACITY : (arg->thread_id+1)*MAP_CAPACITY/WORKERS;
    for (int i=start; i<end; ++i)
    {
        if (arg->map->data[i].val <= MAP_DEPTH)
            dfs(coord_phase1_full.set(arg->map->data[i].key), arg->map->data[i].val);
        if (arg->thread_id==0 && i%(end/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/end);
    }
    return 0;
}

static void fill_prune_table_1(void)
{
    struct coord *c = &coord_phase1;
    struct map *map = fill_prune_map();
    mtx_t mutexes[c->sym->classes];
    for (int i=0; i<c->sym->classes; ++i)
        mtx_init(&mutexes[i], mtx_plain);
    struct fill_prune_table_arg args[WORKERS];
    thrd_t threads[WORKERS];
    for (int i=0; i<WORKERS; ++i)
    {
        args[i].mutexes = mutexes;
        args[i].thread_id = i;
        args[i].c = c;
        args[i].depth = PRUNE_BASE+2;
        args[i].map = map;
        thrd_create(&threads[i], fill_prune_table_dfs, &args[i]);
    }
    for (int i=0; i<WORKERS; ++i)
        thrd_join(threads[i], NULL);
    for (int i=0; i<c->sym->classes; ++i)
        mtx_destroy(&mutexes[i]);
    clear_stderr();
    if (VERBOSE)
        log_dist(c, c->bits, PRUNE_BASE, PRUNE_BASE+4);
}

#define USE_PREPASS 0

// find all solutions to positions in H of 14 moves or less with IDA*
// exit prune
// use a prepass to save time (mostly) on the last iteration
// use a cached prepass
// BFS up to certain depth to minimise repreated symmetric positions
static void fill_prune_table_2(void)
{
    ASSERT(EO_VARIANT == 0);
    struct coord *c = &coord_phase2;

    void prepass(void);

    void dfs(int max_depth)
    {
        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            int in_H = coord_phase1.get(x) == 0;
            if (depth == 1 && move != U && move != U2 ||
                depth == max_depth && !in_H ||
                depth + h(x) > max_depth)
                return;
            if (depth == max_depth)
            {
                for (int s=0; s<NUM_SYMS; s++)
                    if (is_self_sym(c, x, s))
                        TABLE_SET_MIN(c->table, c->bits, c->get(apply_sym(x, s)), depth);
            }
            else
                *top++ = (struct search_node){x, move, depth};
        }

        push(new_cube(), EMPTY_MOVE, 0);
        while (top > stack)
        {
            struct search_node cur = *--top;
            FOREACH_MOVE(cur.move)
                push(apply_move(cur.cube, m), m, cur.depth+1);
        }
    }

    int max_depth = 13;
    for (int depth=0; depth<max_depth; ++depth)
    {
        fprintf(stderr, "\rsearching depth %d...", depth);
        dfs(depth);
    }
    clear_stderr();
    log_dist(c, c->bits, 0, max_depth);
}
