static void build_search_queue(struct queue_node *queue, cube_t x)
{
    struct search_node stack[256];
    struct search_node *top = stack;
    int count = 0;
    unsigned path = 0;

    void push(cube_t x, int move, int depth)
    {
        if (depth <= QUEUE_DEPTH)
            *top++ = (struct search_node){x, move, depth};
    }

    push(x, EMPTY_MOVE, 0);
    while (top>stack)
        {
        struct search_node cur = *--top;
        if (cur.depth)
        {
            int shift = 8*(cur.depth-1);
            unsigned mask = ~(0xffu<<shift);
            path = (path & mask) | cur.move<<shift;
            if (cur.depth==QUEUE_DEPTH)
            {
                queue[count++] = (struct queue_node){cur.cube, path};
                continue;
            }
        }

        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), m, cur.depth+1);
    }

    ASSERT(count == QUEUE_LENGTH);
}

static int search(cube_t x, int *path, int move, int start_depth, int max_depth)
{
    struct search_node
    {
        cube_t cube, inverse;
        int move;
        int depth;
    };

    int min = 20;
    struct search_node stack[256];
    struct search_node *top = stack;

    void push(cube_t x, cube_t y, int move, int depth)
    {
        int f = MAX(h_phase1(x), h_phase1(y)) + depth;
        if (f > max_depth)
            min = MIN(min, f);
        else
            *top++ = (struct search_node){x, y, move, depth};
    }

    push(x, inverse(x), move, start_depth);
    while (top>stack)
    {
        struct search_node cur = *--top;
        if (cur.depth-start_depth > 0)
            path[cur.depth-1] = cur.move;
        if (cube_eq(cur.cube, new_cube()))
            return 0;
        if (cur.depth == max_depth)
            continue;
        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), apply_pre_move(cur.inverse, m), m, cur.depth+1);
    }

    return min-max_depth;
}

static void search_thread(void *__arg)
{
    struct search_arg *arg = __arg;
    for (int i=arg->thread_id; !atomic_load(arg->done) && i<QUEUE_LENGTH; i+=THREADS)
        if (!search(arg->queue[i].cube, arg->path, arg->queue[i].path>>(QUEUE_DEPTH-1)*8, arg->start_depth, arg->max_depth))
        {
            atomic_store(arg->done, 1);
            arg->result = i;
        }
}

static void optimal(cube_t x, int *path, int *length)
{
    *length = 0;
    while (*length < QUEUE_DEPTH)
    {
        int diff = search(x, path, EMPTY_MOVE, 0, *length);
        if (!diff) return;
        *length += diff;
    }

    // TODO increase the maximum depth by whatever `search` returns
    // TODO sort the queue based on a heuristic
    int done = 0;
    struct queue_node queue[QUEUE_LENGTH];
    build_search_queue(queue, x);
    for (;;)
    {
        struct search_arg args[THREADS];
        for (int t=0; t<THREADS; ++t)
        {
            args[t].thread_id = t;
            args[t].queue = queue;
            args[t].start_depth = QUEUE_DEPTH;
            args[t].max_depth = *length;
            args[t].done = &done;
            args[t].result = -1;
        }
        job_dispatch(search_thread, args, sizeof(struct search_arg));
        for (int t=0; t<THREADS; ++t)
        {
            if (args[t].result == -1)
                continue;
            for (int j=0; j<QUEUE_DEPTH; ++j)
                path[j] = args[t].queue[args[t].result].path>>8*j&0xff;
            memcpy(path+QUEUE_DEPTH,
                   args[t].path+QUEUE_DEPTH,
                   sizeof(int)*args[t].max_depth-QUEUE_DEPTH);
        }
        if (done)
            break;
        ++*length;
    }
}
