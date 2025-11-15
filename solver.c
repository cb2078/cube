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

static void n_phase(cube_t x, int *path, int *length, struct coord *coords, int n)
{
    for (int i=0; i<n; ++i)
    {
        init_sym(&coords[i]);
        init_prune_table(&coords[i]);
    }

    *length = 0;
    for (int i=0; i<n; ++i)
    {
        int depth = idA(x, path+*length, coords[i].h, coords[i].move_mask);
        x = apply_moves(x, path+*length, depth);
        printf("phase%d: ", i);
        print_moves(path+*length, depth);
        printf(" // %d move%s\n", depth, depth==1?"":"s");
        *length += depth;
    }
    apply_cancellations(path, length);
    printf("full: ");
    print_moves(path, *length);
    printf(" // %d move%s\n", *length, *length==1?"":"s");
}

static void thistlethwaite(cube_t x, int *path, int *length)
{
    init_tetrad_twist_table();
    n_phase(x, path, length, thistlethwaite_coords, LENGTH(thistlethwaite_coords));
}

static void kociemba(cube_t x, int *path, int *length)
{
    n_phase(x, path, length, kociemba_coords, LENGTH(kociemba_coords));
}

static int hh(cube_t x)
{
    int result = 0;
    result = MAX(result, h_optimal(x));
    result = MAX(result, h_optimal(apply_sym(x, 16)));
    result = MAX(result, h_optimal(apply_sym(x, 32)));
    return result ?: !cube_eq(x, new_cube());
}

static void optimal(cube_t x, int *path, int *length)
{
    init_sym(&optimal_coords[0]);
    init_prune_table(&optimal_coords[0]);
    solve(x, path, length, hh, 0);
    printf("solution: "), print_moves(path, *length), printf(" (%d move%s)\n", *length, *length==1?"":"s");
}
