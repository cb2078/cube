#define STACK_LENGTH 300
#define QUEUE_LENGTH 43254
#define QUEUE_DEPTH 4

static int search(cube_t x, int *path, int (*h)(cube_t), int max_depth)
{
    int min = INT_MAX;
    struct search_node stack[STACK_LENGTH];
    struct search_node *top = stack;

    void push(cube_t x, int move, int depth)
    {
        int f = h(x) + depth;
        if (f > max_depth)
            min = MIN(min, f);
        else
            *top++ = (struct search_node){x, move, depth};
    }

    push(x, EMPTY_MOVE, 0);
    while (top>stack)
    {
        struct search_node cur = *--top;
        if (cur.depth)
            path[cur.depth-1] = cur.move;
        if (0 == h(cur.cube))
            return 0;

        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), m, cur.depth+1);
    }

    return min-max_depth;
}

static void solve(cube_t x, int *path, int *length, int (*h)(cube_t))
{
    int diff;
    *length = 0;
    do
        *length += diff = search(x, path, h, *length);
    while (diff);
}

////////////////////////////////////////////////////////////////////////////////

struct queue_node
{
    cube_t cube;
    unsigned path;
};

static void build_search_queue(struct queue_node *queue, cube_t x)
{
    struct search_node stack[STACK_LENGTH];
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
            if (cur.depth>=4)
            {
                ASSERT(cur.depth==4);
                queue[count++] = (struct queue_node){cur.cube, path};
                continue;
            }
        }

        FOREACH_MOVE(cur.move)
            push(apply_move(cur.cube, m), m, cur.depth+1);
    }

    ASSERT(count == QUEUE_LENGTH);
}

struct search_arg
{
    int (*h)(cube_t);
    int thread_id;
    struct queue_node *queue;
    int path[20];
    int depth;
    int *done;
};

static int search_thread(void *__arg)
{
    struct search_arg *arg = __arg;
    for (int i=arg->thread_id; !atomic_load(arg->done) && i<QUEUE_LENGTH; i+=THREADS)
        if (!search(arg->queue[i].cube, arg->path, arg->h, arg->depth))
        {
            atomic_store(arg->done, 1);
            return i;
        }
    return -1;
}

static void optimal(struct coord *c, cube_t x, int *path, int *length)
{
    init_coord(c);

    *length = 0;
    while (*length < QUEUE_DEPTH)
    {
        int diff = search(x, path, c->h_optimal, *length);
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
        thrd_t threads[THREADS];
        struct search_arg args[THREADS];
        for (int t=0; t<THREADS; ++t)
        {
            args[t].h = c->h_optimal;
            args[t].thread_id = t;
            args[t].queue = queue;
            args[t].depth = *length-QUEUE_DEPTH;
            args[t].done = &done;
            thrd_create(&threads[t], search_thread, &args[t]);
        }
        for (int t=0; t<THREADS; ++t)
        {
            int i;
            thrd_join(threads[t], &i);
            if (i == -1)
                continue;
            for (int j=0; j<QUEUE_DEPTH; ++j)
                path[j] = args[t].queue[i].path>>8*j&0xff;
            memcpy(path+4, args[t].path, sizeof(int)*args[t].depth);
        }
        if (done)
            break;
        ++*length;
    }
}
