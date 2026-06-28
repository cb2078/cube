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

static int search(cube_t x, int *path, int start_depth, int max_depth)
{
    struct search_node
    {
        cube_t cube, inverse;
        int move;
        int depth;
    };

    int min = INT_MAX;
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

    push(x, inverse(x), EMPTY_MOVE, start_depth);
    while (top>stack)
    {
        struct search_node cur = *--top;
        if (cur.depth-start_depth > 0)
            path[cur.depth-1] = cur.move;
        if (cube_eq(cur.cube, new_cube()))
            return 0;
        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), apply_pre_move(cur.inverse, m), m, cur.depth+1);
    }

    return min-max_depth;
}

static int search_thread(void *__arg)
{
    struct search_arg *arg = __arg;
    for (int i=arg->thread_id; !atomic_load(arg->done) && i<QUEUE_LENGTH; i+=WORKERS)
        if (!search(arg->queue[i].cube, arg->path, arg->start_depth, arg->max_depth))
        {
            atomic_store(arg->done, 1);
            return i;
        }
    return -1;
}

static void optimal(cube_t x, int *path, int *length)
{
    *length = 0;
    while (*length < QUEUE_DEPTH)
    {
        int diff = search(x, path, 0, *length);
        if (!diff) return;
        *length += diff;
    }

    // TODO increase the maximum depth by whatever `search` returns
    // TODO sort the queue based on a heuristic
    int done = 0;
    struct queue_node queue[QUEUE_LENGTH];
    build_search_queue(queue, x);
    while (*length<=20)
    {
        thrd_t threads[WORKERS];
        struct search_arg args[WORKERS];
        for (int w=0; w<WORKERS; ++w)
        {
            args[w].thread_id = w;
            args[w].queue = queue;
            args[w].start_depth = QUEUE_DEPTH;
            args[w].max_depth = *length;
            args[w].done = &done;
            // TODO if we know the last move of the queue search, can we prune
            // canonical sequances immediately after starting the search?
            thrd_create(&threads[w], search_thread, &args[w]);
        }
        for (int w=0; w<WORKERS; ++w)
        {
            int i;
            thrd_join(threads[w], &i);
            if (i == -1)
                continue;
            for (int j=0; j<QUEUE_DEPTH; ++j)
                path[j] = args[w].queue[i].path>>8*j&0xff;
            memcpy(path+QUEUE_DEPTH,
                   args[w].path+QUEUE_DEPTH,
                   sizeof(int)*args[w].max_depth-QUEUE_DEPTH);
        }
        if (done)
            break;
        ++*length;
    }
}
