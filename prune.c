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
    map_set(map, get_phase1_full(new_cube()), 0);
    for (int depth=1; depth<=MAP_DEPTH; ++depth)
        for (long long i=0; i<MAP_CAPACITY; ++i)
        {
            if (map->data[i].val == depth-1)
                for (int m=0; m<18; ++m)
                {
                    cube_t x = apply_move(set_phase1_full(map->data[i].key), m);
                    for (int s=0; s<NUM_SYMS; ++s)
                    {
                        if (!is_self_sym(c, x, s))
                            continue;
                        long long k = get_phase1_full(apply_sym(x, s));
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

static void fill_prune_table_dfs(void *_)
{
    struct fill_prune_table_arg *arg = _;
    struct coord *c = &coord_phase1;

    void dfs(cube_t x, int start_depth)
    {
        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            int class = get_phase1(x)%c->sym->classes;
            mtx_lock(&arg->mutexes[class]);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                if (!is_self_sym(c, x, s))
                    continue;
                int v = MAX(depth-PRUNE_BASE, 0);
                long long i = get_phase1(apply_sym(x, s));
                long long j = PRUNE_EXT_62(i);
                long long k = PRUNE_MIN_62(i);
                TABLE_SET_MIN(c->table, 2, j, v);
                TABLE_SET_MIN(c->table, 4, k/2, depth);
            }
            mtx_unlock(&arg->mutexes[class]);
            if (depth >= MAP_DEPTH &&
                depth < arg->depth &&
                depth <= map_get(arg->map, get_phase1_full(x)))
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
            dfs(set_phase1_full(arg->map->data[i].key), arg->map->data[i].val);
        if (arg->thread_id==0 && i%(end/10000)==0)
            fprintf(stderr, "\rcompletion=%.2f%%", 100.0*i/end);
    }
}

static void fill_prune_table_1(void)
{
    struct coord *c = &coord_phase1;
    struct map *map = fill_prune_map();
    mtx_t mutexes[c->sym->classes];
    for (int i=0; i<c->sym->classes; ++i)
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
    for (int i=0; i<c->sym->classes; ++i)
        mtx_destroy(&mutexes[i]);
    clear_stderr();
    if (VERBOSE)
        log_dist(c, c->bits, PRUNE_BASE, PRUNE_BASE+4);
}

// find all solutions to positions in H of 14 moves or less with IDA*
// exit prune
// use a prepass to save time (mostly) on the last iteration
// BFS up to certain depth to minimise repreated symmetric positions
// multi threding
static void fill_prune_table_2(void)
{
    ASSERT(EO_VARIANT == 0);
    struct coord *c = &coord_phase2;
    long long visits = 0;
    long long queue_visits = 1;
    int queue_depth = 5;
    struct queue q = queue_new(1<<14);

    mtx_t mutexes[1172];
    static_assert(ORBIT_CLASSES%LENGTH(mutexes) == 0);
    for (int i=0; i<LENGTH(mutexes); i++)
        mtx_init(&mutexes[i], mtx_plain);

    void visit(cube_t x, int depth)
    {
        if (!in_H(x))
            return;
        long long class = c->sym->info[get_orbit(x)].class;
        long long i = class/(ORBIT_CLASSES/LENGTH(mutexes));
        mtx_lock(&mutexes[i]);
        for (int s=0; s<NUM_SYMS; s++)
            if (is_self_sym(c, x, s))
                TABLE_SET_MIN(c->table, c->bits, get_phase2(apply_sym(x, s)), depth);
        mtx_unlock(&mutexes[i]);
    }

    void bfs(int depth)
    {
        int n = q.length;
        while (n--)
        {
            struct search_node cur = queue_pop(&q);
            visits++;
            visit(cur.cube, depth);
            FOREACH_MOVE(EMPTY_MOVE)
            {
                int s;
                cube_t x = cube_canonicalise(apply_move(cur.cube, m), &s);
                ASSERT(cube_eq(apply_move(apply_sym(cur.cube, s), sym_moves[s][m]), x));
                queue_push(&q, x, sym_moves[s][m], depth+1);
            }
        }
    }

    void dfs(cube_t x, int move, int start_depth, int max_depth)
    {
        struct search_node stack[256];
        struct search_node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            if (depth == max_depth && !in_H(x) ||
#if USE_PREPASS
                depth > start_depth && depth < max_depth && in_H(x) ||
#endif
                depth + h_phase1(x) > max_depth)
                return;
            visits++;
            if (depth == max_depth)
                visit(x, depth);
            else
                *top++ = (struct search_node){x, move, depth};
        }

        // use the last move from the BFS iteration to prune some sequences
        // this move has to be on either the U, R or F faces so sequences starting with a turn of the opposite face don't get pruned
        if (move != EMPTY_MOVE && move_side(move)>0)
        {
            int s = 11;
            x = apply_sym(x, s);
            move = sym_moves[s][move];
        }
        push(x, move, start_depth);
        while (top > stack)
        {
            struct search_node cur = *--top;
            FOREACH_MOVE(cur.move)
                push(apply_move(cur.cube, m), m, cur.depth+1);
        }
    }

    void prepass(int max_depth)
    {
        for (long long i=0; i<c->max; i++)
        {
            int depth = table_get(c->table, c->bits, i);
            if (depth > queue_depth &&
                depth + 5 <= max_depth)
                dfs(set_phase2(i), EMPTY_MOVE, depth, max_depth);
            else if (depth == max_depth-1)
                for (int m=U2; m<=B2; m++)
                {
                    cube_t x = apply_move(set_phase2(i), m);
                    ASSERT(in_H(x));
                    visits++;
                    visit(x, max_depth);
                }
        }
    }

    queue_push(&q, new_cube(), EMPTY_MOVE, 0);
    int max_depth = MAX_DEPTH;
    for (int depth=0; depth<max_depth; ++depth)
        if (depth<queue_depth)
        {
            fprintf(stderr, "\r      bfs depth %d...", depth);
            bfs(depth);
            queue_visits += q.length;
        }
        else
        {
            void dfs_parallel(void *varg)
            {
                int thread_id = (long long)varg;
                for (int i=thread_id; i<q.length; i+=THREADS)
                    dfs(queue_get(&q, i).cube, queue_get(&q, i).move, queue_depth, depth);
            }

#if USE_PREPASS
            fprintf(stderr, "\r  prepass depth %d...", depth);
            prepass(depth);
#endif
            fprintf(stderr, "\rsearching depth %d...", depth);
            job_dispatch(dfs_parallel, 0, 0);
        }
    for (int i=0; i<LENGTH(mutexes); i++)
        mtx_destroy(&mutexes[i]);
    clear_stderr();
    log_dist(c, c->bits, 0, max_depth);
    printf("visits=%lld\n", visits);
    long long canonical_sequences[] =
    {
        1,
        18,
        243,
        3240,
        43254,
        577368,
    };
    printf("queue_visits=%lld (ratio=%.1f)\n", queue_visits, (float)canonical_sequences[queue_depth]/queue_visits);
}