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

static void search_thread(void *_)
{
    struct search_arg *arg = _;
    int path[20];
    for (int i; !atomic_load(arg->done) && (i=atomic_fetch_add(arg->start, 1)) < QUEUE_LENGTH;)
        if (!search(arg->queue[i].cube, path, arg->queue[i].path>>(QUEUE_DEPTH-1)*8, QUEUE_DEPTH, arg->depth))
        {
            // check if another thread has also got here
            if (atomic_exchange(arg->done, 1) != 0)
                break;
            for (int j=0; j<QUEUE_DEPTH; ++j)
                arg->path[j] = arg->queue[i].path>>8*j&0xff;
            memcpy(arg->path+QUEUE_DEPTH, path+QUEUE_DEPTH, sizeof(int)*arg->depth-QUEUE_DEPTH);
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

    // TODO sort the queue based on a heuristic
    struct queue_node queue[QUEUE_LENGTH];
    build_search_queue(queue, x);
    for (int done=0; !done; *length+=!done)
    {
        int start = 0;
        struct search_arg args[THREADS];
        for (int t=0; t<THREADS; ++t)
        {
            args[t].start = &start;
            args[t].queue = queue;
            args[t].path = path;
            args[t].depth = *length;
            args[t].done = &done;
        }
        job_dispatch(search_thread, args, sizeof(struct search_arg));
    }
}
