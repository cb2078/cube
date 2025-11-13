#define H(name)\
    static int h_##name(cube x)\
    {\
        return table_get(coord_##name.table, coord_get(&coord_##name, x));\
    }
H(stage1);
H(stage2);
H(stage3);
H(stage4);
H(phase1);
H(phase2);
H(optimal);

static int idA(cube x, int *path, int (*h)(cube), int move_mask)
{
    typedef struct
    {
        cube cube;
        int move;
        int depth;
    } node;

    for (int max_depth=0;;)
    {
        int min = INT_MAX;
        node stack[20*15];
        node *top = stack;

        void push(cube x, int move, int depth)
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

static void solve(cube x, int *path, int *length, int (*h)(cube), int move_mask)
{
    *length = idA(x, path, h, move_mask);
}

struct stage
{
    struct coord *coord;
    int (*h)(cube);
};

static void n_stage(cube x, int *path, int *length, struct stage *stages, int n)
{
    for (int i=0; i<n; ++i)
    {
        init_sym(stages[i].coord);
        init_prune_table(stages[i].coord);
    }

    *length = 0;
    for (int i=0; i<n; ++i)
    {
        int depth = idA(x, path+*length, stages[i].h, stages[i].coord->move_mask);
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

static void thistlethwaite(cube x, int *path, int *length)
{
    static struct stage stages[] =
    {
        {&coord_stage1, h_stage1},
        {&coord_stage2, h_stage2},
        {&coord_stage3, h_stage3},
        {&coord_stage4, h_stage4},
    };
    init_tetrad_twist_table();
    n_stage(x, path, length, stages, LENGTH(stages));
}

static void kociemba(cube x, int *path, int *length)
{
    static struct stage stages[] =
    {
        {&coord_phase1, h_phase1},
        {&coord_phase2, h_phase2},
    };
    n_stage(x, path, length, stages, LENGTH(stages));
}

static void optimal(cube x, int *path, int *length)
{
    static struct stage stages[] =
    {
        {&coord_optimal, h_optimal},
    };
    n_stage(x, path, length, stages, LENGTH(stages));
}
