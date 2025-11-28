static int idA(cube_t x, int *path, int (*h)(cube_t), int move_mask)
{
    typedef struct
    {
        cube_t cube;
        int move;
        int depth;
    } node;

    for (int max_depth=0;;)
    {
        int min = INT_MAX;
        node stack[20*15];
        node *top = stack;

        void push(cube_t x, int move, int depth)
        {
            int f = h(x) + depth;
            if (f > max_depth)
                min = MIN(min, f);
            else
                *top++ = (node){x, move, depth};
        }

        push(x, 0xff, 0);
        while (top>stack)
        {
            node cur = *--top;
            if (cur.depth)
                path[cur.depth-1] = cur.move;
            if (0 == h(cur.cube))
                return cur.depth;

            int moves[18], length;
            possible_moves(moves, &length, cur.move, move_mask);
            for (int i=0; i<length; ++i)
                push(apply_move(cur.cube, moves[i]), moves[i], cur.depth+1);
        }

        max_depth = min;
    }
}

static void solve(cube_t x, int *path, int *length, int (*h)(cube_t), int move_mask)
{
    *length = idA(x, path, h, move_mask);
}

static int hh(cube_t x)
{
    return h_optimal(x) ?: !cube_eq(x, new_cube());
}

static void optimal(cube_t x, int *path, int *length)
{
    init_coord(&coord_optimal);
    solve(x, path, length, hh, coord_optimal.move_mask);
}
