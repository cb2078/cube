static void clear_stderr(void)
{
    fprintf(stderr, "\r");
    for (int i=0; i<80; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\r");
}

static void log_dist(unsigned *table, long long max, int bits, int base, int max_depth)
{
    if (!VERBOSE)
        return;
    long long dist[21] = {0}, sum = 0;
    for (long long i=0; i<max; ++i)
        dist[table_get(table, bits, i)]++;
    for (int i=base; i<max_depth; ++i)
    {
        LOG("%s[%d] = %lld", i<10?" ":"", i, dist[i-base]);
        if (i>base)
            LOG("\t(%.1fx)\n", (float)dist[i-base]/dist[i-1-base]);
        else
            LOG("\n");
        sum+=dist[i-base];
    }
    LOG("skipped %lld entries (%.2f%% filled)\n", max-sum, 100.0*(sum)/max);
}

static void fill_sym_table(void)
{
    for (long long i=0; i<SYM_COORD_MAX; ++i)
    {
        cube_t x = set_sym_coord(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            long long k = get_sym_coord(apply_sym(x, s));
            sym_coord.self_syms[i] |= (long long)(i==k)<<s%48;
        }
        if (i%(SYM_COORD_MAX/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/SYM_COORD_MAX);
    }
    clear_stderr();

    int class = 0;
    memset(sym_coord.info, 0xff, 4*SYM_COORD_MAX);
    for (long long i=0; i<SYM_COORD_MAX; ++i)
    {
        if (*(unsigned *)&sym_coord.info[i] != -1u)
            continue;
        cube_t x = set_sym_coord(i);
        for (int s=0; s<NUM_SYMS; ++s)
        {
            int k = get_sym_coord(apply_sym(x, s));
            if (s && k==i)
                continue;
            sym_coord.info[k].class = class;
            sym_coord.info[k].sym = inv_sym[s];
        }
        sym_coord.to_rep[class++] = i;
        if (i%(SYM_COORD_MAX/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/SYM_COORD_MAX);
    }
    clear_stderr();
    LOG("%s classes : %d\n", SYM_COORD_NAME, class);
    ASSERT(class == SYM_COORD_CLASSES);
}

static struct map *fill_prune_map(void)
{
    struct map *map = map_new();
    map_set(map, get_coord(new_cube(), 11), EMPTY_MOVE, 0);
    for (int depth=1; depth<=MAP_DEPTH; ++depth)
        for (long long i=0; i<MAP_CAPACITY; ++i)
        {
            if (map->data[i].val == depth-1)
                for (int m=0; m<18; ++m)
                {
                    cube_t x = apply_move(set_coord(map->data[i].key, 11), m);
                    for (int s=0; s<NUM_SYMS; ++s)
                    {
                        if (!is_self_sym(x, s))
                            continue;
                        cube_t y = apply_sym(x, s);
                        long long k = get_coord(y, 11);
                        if (map_get(map, k) == MAP_VAL_MAX)
                        {
                            // transform the move by the symmetry
                            int move = sym_moves[s][m];
                            int sym = sym_coord.info[get_sym_coord(y)].sym;
                            // transform the move by the symmetry the brings y to its representant
                            map_set(map, k, sym_moves[sym][move], depth);
                        }
                    }
                }
            if ((i+MAP_CAPACITY*(depth-1))%(MAP_DEPTH*MAP_CAPACITY/1000)==0)
                fprintf(stderr, "\rdepth=%d load=%.1f%% mem=%.1fMB",
                        depth, 100.0*map->count/MAP_CAPACITY, 8.0*map->count/1e6);
        }
    clear_stderr();
    return map;
}

static void fill_prune_table_dfs(void *_)
{
    struct fill_prune_table_arg *arg = _;

    void dfs(cube_t x, int move, int start_depth)
    {
        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            int class = get_coord(x, EO_VARIANT)/(ESEP_MAX<<EO_VARIANT);
            mtx_lock(&arg->mutexes[class]);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                if (!is_self_sym(x, s))
                    continue;
                int v = MAX(depth-PRUNE_BASE, 0);
                long long i = get_coord(apply_sym(x, s), EO_VARIANT);
                long long j = PRUNE_EXT_62(i);
                long long k = PRUNE_MIN_62(i);
                TABLE_SET_MIN(coord.table, 2, j, v);
                TABLE_SET_MIN(coord.table, 4, k/2, depth);
            }
            mtx_unlock(&arg->mutexes[class]);
            if (depth >= MAP_DEPTH &&
                depth < arg->depth &&
                depth <= map_get(arg->map, get_coord(x, 11)))
                *top++ = (struct search_node){x, move, depth};
        }
        
        ASSERT(move != EMPTY_MOVE);
        if (move_side(move)>0)
        {
            int s = 11;
            x = apply_sym(x, s);
            move = sym_moves[s][move];
        }
        push(x, move, start_depth);
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
            dfs(set_coord(arg->map->data[i].key, 11), arg->map->data[i].move, arg->map->data[i].val);
        if (arg->thread_id==0 && i%(end/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/end);
    }
}

static void fill_prune_table(void)
{
    struct map *map = fill_prune_map();
    mtx_t mutexes[SYM_COORD_CLASSES];
    for (int i=0; i<SYM_COORD_CLASSES; ++i)
        mtx_init(&mutexes[i], mtx_plain);
    struct fill_prune_table_arg args[THREADS];
    for (int i=0; i<THREADS; ++i)
    {
        args[i].mutexes = mutexes;
        args[i].thread_id = i;
        args[i].depth = PRUNE_BASE+2;
        args[i].map = map;
    }
    job_dispatch(fill_prune_table_dfs, args, sizeof(struct fill_prune_table_arg));
    for (int i=0; i<SYM_COORD_CLASSES; ++i)
        mtx_destroy(&mutexes[i]);
    clear_stderr();
    if (VERBOSE)
        log_dist(coord.table, COORD_MAX, coord.bits, PRUNE_BASE, PRUNE_BASE+4);
}