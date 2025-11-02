static cube new_cube(void)
{
    cube x;
    for (int i=0; i<NUM_CORNERS; ++i) x.corners[i]=i;
    for (int i=0; i<NUM_EDGES; ++i) x.edges[i]=i;
    return x;
}

static void print_cube(cube x)
{
    #define PRINT(x, ...) \
        printf("%2s ", x); \
        for (int i=0; i<NUM_CORNERS+NUM_EDGES; ++i) \
            printf(__VA_ARGS__); \
        printf("\n");
    PRINT("",   "%s%3s", i?" ":"", cubie_str[i]);
    PRINT("p:", "%s%3d", i?",":"", x.cubies[i]&0x0f);
    PRINT("o:", "%s%3d", i?",":"", x.cubies[i]>>4);
    #undef PRINT
}

static int cube_eq(cube x, cube y)
{
    return 0==memcmp(&x, &y, sizeof(cube));
}

static cube compose(cube x, cube y)
{
    cube result;

    for (int i=0; i<NUM_CORNERS; ++i)
    {
        result.corners[i] = x.corners[y.corners[i]&0x0f];
        result.corners[i] += y.corners[i]&0xf0;
        result.corners[i] %= 3*0x10;
    }

    for (int i=0; i<NUM_EDGES; ++i)
    {
        result.edges[i] = x.edges[y.edges[i]&0x0f];
        result.edges[i] += y.edges[i]&0xf0;
        result.edges[i] %= 2*0x10;
    }

    return result;
}

static cube compose_3(cube x, cube y, cube z)
{
    return compose(compose(x, y), z);
}

static cube apply_sym(cube x, int sym)
{
    x = compose_3(inv_sym_table[sym], x, sym_table[sym]);
    if (sym&1) x = invert_co(x);
    return x;
}

static cube apply_move(cube x, int move)
{
    return compose(x, move_table[move]);
}

static cube apply_moves(cube x, int *moves, int length)
{
    cube result=x;
    for (int i=0; i<length; ++i) result=apply_move(result, moves[i]);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

static void orient(char *x, int orientation)
{
    assert(orientation<3);
    *x = (*x&0x0f) | (orientation<<4);
}

static int get_eo(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_EDGES-1; ++i)
        result += x.edges[i]>>4<<i;
    return result;
}

static void set_eo(cube *x, long long r)
{
    int parity = 0;
    for (int i=0, y; i<NUM_EDGES-1; ++i, r>>=1)
        orient(x->edges+i, y=r&1), parity^=y;
    orient(x->edges+NUM_EDGES-1, parity);
}

static int get_co(cube x)
{
    int result = 0;
    for (int i=0; i<NUM_CORNERS-1; ++i)
        result += (x.corners[i]>>4)*pow3[i];
    return result;
}

static void set_co(cube *x, long long r)
{
    int parity = 0;
    for (int i=0, y; i<NUM_CORNERS-1; ++i, r/=3)
        orient(x->corners+i, y=r%3), parity+=y;
    orient(x->corners+NUM_CORNERS-1, (3-parity%3)%3);
}

static table *tetrad_twist_table;
static int get_tetrad_twist(cube x)
{
    return table_get(tetrad_twist_table, get_permutation(x.corners, NUM_CORNERS));
}

static void set_tetrad_twist(cube *x, int r)
{
    char perm[3];
    set_permutation(perm, 3, r);
    for (int i=0; i<NUM_CORNERS; ++i)
        if (x->corners[i] < 3)
            x->corners[i] = perm[(int)x->corners[i]];
}

static int h_cp5(cube x)
{
    for (int i=3; i<NUM_CORNERS; ++i)
        if (x.corners[i]!=i)
            return 1;
    return 0;
}

static cube invert_co(cube x)
{
    for (int i=0; i<NUM_CORNERS; ++i)
    {
        int co = x.corners[i]>>4;
        orient(x.corners+i, co?3-co:co);
    }
    return x;
}

///////////////////////////////////////////////////////////////////////////////

#define STACK_SIZE 2048
#define PUSH(cube, move, depth) (assert(top-stack<STACK_SIZE), *top++ = (node){(cube), (move), (depth)})

typedef struct
{
    cube cube;
    int move;
    int depth;
} node;

static int idA(cube x, int *path, int (*h)(cube), int quater_turns[6])
{
    for (int max_depth=0;;)
    {
        int min = INT_MAX;
        node stack[STACK_SIZE];
        node *top = stack;
        PUSH(x, 0xff, 0);
        while (top>stack)
        {
            node cur = *--top;

            int f = h(cur.cube) + cur.depth;
            if (f > max_depth)
            {
                min = f<min ? f : min;
                continue;
            }
            if (cur.depth)
                path[cur.depth-1] = cur.move;
            if (0 == h(cur.cube))
                return cur.depth;

            int moves[18];
            int length;
            possible_moves(moves, &length, cur.move, quater_turns);
            for (int i=0; i<length; ++i)
                PUSH(apply_move(cur.cube, moves[i]), moves[i], cur.depth+1);
        }
        max_depth = min;
    }
}

static void solve(cube x, int *path, int *length, int (*h)(cube), int quater_turns[6])
{
    *length = idA(x, path, h, quater_turns);
}

///////////////////////////////////////////////////////////////////////////////

static void init_sym(coord *c)
{
    if (!c->is_sym)
        return;

    // since we're only dealing with the symmetry part of the coordinate, rename some of its variables
    long long *order = &c->sym_part_order;
    long long *eqv_classes = &c->order;
    long long (*get)(cube) = c->get_sym_part;
    cube (*set)(long long) = c->set_sym_part;

    for (long long i=0; i<*order; ++i)
        c->coord_to_eqv_class[i] = *order;
    for (long long i=0; i<*order; ++i)
    {
        if (c->coord_to_eqv_class[i]<*order)
            continue;
        cube x = set(i);
        for (int j=0; j<c->num_syms; ++j)
        {
            cube y = apply_sym(x, j);
            int k = get(y);
            c->coord_to_eqv_class[k] = *eqv_classes;
            c->coord_to_rep_sym[k] = inv_sym[j];
        }
        c->eqv_class_to_rep[(*eqv_classes)++] = i;
    }
}

static void init_prune_table(coord *c)
{
    if (table_read(c->table = table_new(c->order, 4, c->name)))
        return;

    void print(long long n, int depth, int reverse)
    {
        fprintf(stderr, "\rdepth=%d comletion=%.2f%%", depth, (double)n/c->order*100);
        if (reverse)
            fprintf(stderr, " (backsearch)");
    }

    long long distribution[20] = {1};
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    long long n = 1;
    for (int depth=0, x=0, reverse=0; n<c->order && depth<c->table->mask; ++depth)
    {
        long long m = n;
        for (long long i=0; i<c->order; ++i)
            if (!reverse && c->table->data[i/c->table->divisor] == UINT_MAX)
            {
                i += c->table->divisor-1;
                continue;
            }
            else if (table_get(c->table, i) == (reverse ? c->table->mask : depth))
            {
                int moves[18], length;
                possible_moves(moves, &length, 0xff, c->quater_turns);
                for (int j=0; j<length; ++j)
                {
                    long long k = c->get(apply_move(c->set(i), moves[j]));
                    if (!reverse && table_get(c->table, k) == c->table->mask)
                        table_set(c->table, k, depth+1);
                    else if (reverse && table_get(c->table, k) == depth)
                        table_set(c->table, i, depth+1);
                    else
                        continue;
                    if (++n*10000/c->order > x)
                        print(n, depth, reverse), ++x;
                    if (reverse)
                        break;
                }
            }
        reverse = n>c->order/2;
        print(n, depth, reverse);
        distribution[depth+1] = n-m;
    }
    fprintf(stderr, "\r                                           \r");
    if (n!=c->order)
        printf("skpped %lld entries\n", c->order-n);
    printf("%s distribution:\n", c->name);
    for (int i=0; i<LENGTH(distribution); ++i)
        if (distribution[i])
            printf("%s[%d] = %lld\n", i<10?" ":"", i, distribution[i]);

    table_write(c->table);
}

static void init_tetrad_twist_table(void)
{
    cube separate_corners(cube x)
    {
        cube result = x;
        int j=0, k=0;
        for (int i=0; i<NUM_CORNERS; ++i)
            if (x.corners[i]<4)
                result.tetrads[0][j++] = x.corners[i];
            else
                result.tetrads[1][k++] = x.corners[i];
        assert(j==4 && k==4);
        return result;
    }

    int n = fact[8];
    tetrad_twist_table = table_new(n, 4, "tetrad-twist");
    if (table_read(tetrad_twist_table)) return;

    for (int i=0; i<n; ++i)
    {
        cube x = new_cube();
        set_permutation(x.corners, NUM_CORNERS, i);
        x = separate_corners(x);
        int moves[64], length;
        solve(x, moves, &length, h_cp5, tw_coords[LENGTH(tw_coords)-1].quater_turns); // todo half turns
        x = apply_moves(x, moves, length);
        table_set(tetrad_twist_table, i, get_permutation(x.corners, 3));
        fprintf(stderr, "\rcompletion=%.2f%%", (float)i/n*100);
    }
    fprintf(stderr, "\r                                        \r");

    table_write(tetrad_twist_table); // todo error handling
}

////////////////////////////////////////////////////////////////////////////////

static void thistlethwaite(cube x, int *path, int *length)
{
    static int initialised = 0;
    if (!initialised)
    {
        init_tetrad_twist_table();
        for (int i=0; i<LENGTH(tw_coords); ++i)
        {
            init_sym(&tw_coords[i]);
            init_prune_table(&tw_coords[i]);
        }
        initialised = 1;
    }

    *length = 0;
    for (int i=0; i<LENGTH(tw_coords); ++i)
    {
        int depth = idA(x, path+*length, tw_coords[i].h, tw_coords[i].quater_turns);
        x = apply_moves(x, path+*length, depth);
        printf("stage%d: ", i);
        print_moves(path+*length, depth);
        printf(" // %d move%s\n", depth, depth==1?"":"s");
        *length += depth;
    }
    apply_cancellations(path, length);
    printf("full: ");
    print_moves(path, *length);
    printf(" // %d move%s\n", *length, *length==1?"":"s");
}
