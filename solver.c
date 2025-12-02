#define STACK_LENGTH 300
#define QUEUE_LENGTH 43254
#define QUEUE_DEPTH 4

struct search_node
{
    cube_t cube;
    int move;
    int depth;
};

static int search(cube_t x, int *path, int (*h)(cube_t), int move_mask, int max_depth)
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

    push(x, 0xff, 0);
    while (top>stack)
    {
        struct search_node cur = *--top;
        if (cur.depth)
            path[cur.depth-1] = cur.move;
        if (0 == h(cur.cube))
            return 0;

        int moves[18], length;
        possible_moves(moves, &length, cur.move, move_mask);
        for (int i=0; i<length; ++i)
            push(apply_move(cur.cube, moves[i]), moves[i], cur.depth+1);
    }

    return min-max_depth;
}

static void solve(cube_t x, int *path, int *length, int (*h)(cube_t), int move_mask)
{
    int diff;
    *length = 0;
    do
        *length += diff = search(x, path, h, move_mask, *length);
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

    push(x, 0xff, 0);
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

        int moves[18], length;
        possible_moves(moves, &length, cur.move, 0);
        for (int i=0; i<length; ++i)
            push(apply_move(cur.cube, moves[i]), moves[i], cur.depth+1);
    }

    ASSERT(count == QUEUE_LENGTH);
}

static int hh(cube_t x)
{
    return h_optimal(x) ?: !cube_eq(x, new_cube());
}

struct search_arg
{
    int thread_id;
    struct queue_node *queue;
    int path[20];
    int depth;
    int *done;
};

// TODO have threads pop form a work queue to keep all threads busy
static int search_thread(void *__arg)
{
    struct search_arg *arg = __arg;
    for (int i=arg->thread_id; !atomic_load(arg->done) && i<QUEUE_LENGTH; i+=THREADS)
        if (!search(arg->queue[i].cube, arg->path, hh, 0, arg->depth))
        {
            atomic_store(arg->done, 1);
            return i;
        }
    return -1;
}

static void optimal(cube_t x, int *path, int *length)
{
    init_coord(&coord_optimal);

    *length = 0;
    while (*length < QUEUE_DEPTH)
    {
        int diff = search(x, path, hh, 0, *length);
        if (!diff) return;
        *length += diff;
    }

    // TODO increase the maximum depth by whatever `search` returns
    // TODO sort the queue based on a heuristic
    struct queue_node queue[QUEUE_LENGTH];
    build_search_queue(queue, x);
    for (; *length<=20; ++*length)
    {
        thrd_t threads[THREADS];
        struct search_arg args[THREADS];
        int done = 0;
        for (int t=0; t<THREADS; ++t)
        {
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
            // copy the moves to the depth-4 node to the path
            for (int j=0; j<QUEUE_DEPTH; ++j)
                // TODO use 8 bits for the path
                path[j] = args[t].queue[i].path>>8*j&0xff;
            // copy the moves to from depth-4 node to the path
            memcpy(path+4, args[t].path, sizeof(int)*args[t].depth);
            return;
        }
    }

    UNREACHABLE();
}
