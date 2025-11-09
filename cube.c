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
    // TODO try creating a "mirroed compose" function to see if its faster than
    // inverting the CO twice

    cube maybe_invert_co(cube x)
    {
        return sym&1 ? invert_co(x) : x;
    }

    return compose_3(inv_sym_table[sym], maybe_invert_co(x), maybe_invert_co(sym_table[sym]));
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

///////////////////////////////////////////////////////////////////////////////

static void init_sym(coord *c)
{
    if (!c->is_sym)
        return;

    for (long long i=0; i<c->sym_part_order; ++i)
    {
        cube x = c->set_sym_part(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube y = apply_sym(x, s);
            int k = c->get_sym_part(y);
            c->self_syms[i] |= (i==k)<<s;
        }
    }

    long long eqv_classes = 0;
    memset(c->coord_to_eqv_class, 0xff, c->sym_part_order*sizeof(c->coord_to_eqv_class[0]));
    for (long long i=0; i<c->sym_part_order; ++i)
    {
        if ((unsigned long long)c->coord_to_eqv_class[i] != -1ull)
            continue;
        cube x = c->set_sym_part(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube y = apply_sym(x, s);
            int k = c->get_sym_part(y);
            c->coord_to_eqv_class[k] = eqv_classes;
            c->coord_to_rep[k] = i;
            c->coord_to_rep_sym[k] = inv_sym[s];
        }
        c->eqv_class_to_rep[eqv_classes++] = i;
    }
    if (!c->eqv_classes)
    {
        printf("num eqv classes = %lld\n", eqv_classes);
        c->order *= eqv_classes;
    }
    else
    {
        assert(eqv_classes == c->eqv_classes);
    }
}

static void init_prune_table(coord *c)
{
    void print(long long n, int depth, int backsearch)
    {
        fprintf(stderr, "\rdepth=%d comletion=%.2f%%", depth, (double)n/c->order*100);
        if (backsearch)
            fprintf(stderr, " (backsearch)");
    }

    void clear(void)
    {
        fprintf(stderr, "\r                                           \r");
    }

    if (table_read(c->table = table_new(c->order, 4, c->name)))
        return;
    long long n = 1;
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    for (int depth=1, t=0, backsearch=0; c->table->count<c->order && depth<c->table->mask; ++depth)
    {
        long long m = c->table->count;
        for (long long i=0; i<c->order; ++i)
        {
            if (!backsearch && c->table->data[i/c->table->divisor] == UINT_MAX)
            {
                i += c->table->divisor-1;
                continue;
            }
            else if (table_get(c->table, i) == (backsearch ? c->table->mask : depth-1))
            {
                int moves[18], length;
                possible_moves(moves, &length, 0xff, c->move_mask);
                for (int j=0; j<length; ++j)
                {
                    cube x = apply_move(c->set(i), moves[j]);
                    long long k = c->get(x);
                    if (!backsearch && table_get(c->table, k) == c->table->mask)
                    {
                        table_set(c->table, k, depth);
                        if (!c->is_sym)
                            continue;
                        for (int s=1; s<c->num_syms; ++s)
                        {
                            if (~c->self_syms[c->get_sym_part(x)]>>s&1)
                                continue;
                            cube y = apply_sym(x, s);
                            int l = c->get(y);
                            if (table_get(c->table, l) == c->table->mask)
                                table_set(c->table, l, depth);
                        }
                    }
                    else if (backsearch && table_get(c->table, k) == depth-1)
                    {
                        table_set(c->table, i, depth);
                        break;
                    }
                }
            }
            if (c->table->count*10000/c->order > t)
                print(c->table->count, depth, backsearch), ++t;
        }
        backsearch = c->table->count>c->order/2;
        print(c->table->count, depth, backsearch);
        clear();
        printf("[%d]%s= %lld\n", depth, depth<10?"  ":" ", c->table->count-m);
    }
    clear();
    if (c->table->count!=c->order)
        printf("skpped %lld entries\n", c->order-n);
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
        solve(x, moves, &length, h_cp5, HTR_MASK);
        x = apply_moves(x, moves, length);
        table_set(tetrad_twist_table, i, get_permutation(x.corners, 3));
        fprintf(stderr, "\rcompletion=%.2f%%", (float)i/n*100);
    }
    fprintf(stderr, "\r                                        \r");

    table_write(tetrad_twist_table); // todo error handling
}

static void init_coords(coord *coords, int n)
{
    for (int i=0; i<n; ++i)
    {
        init_sym(&coords[i]);
        init_prune_table(&coords[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////

static void n_phase(cube x, int *path, int *length, coord *coords, int n)
{
    init_coords(coords, n);

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

static void thistlethwaite(cube x, int *path, int *length)
{
    init_tetrad_twist_table();
    n_phase(x, path, length, tw_coords, LENGTH(tw_coords));
}

static void two_phase(cube x, int *path, int *length)
{
    init_tetrad_twist_table();
    n_phase(x, path, length, ko_coords, LENGTH(ko_coords));
}

static int h3(cube x)
{
    int result = 0;
    result = MAX(result, ko_coords[0].h(x));
    result = MAX(result, ko_coords[0].h(apply_sym(x, 16)));
    result = MAX(result, ko_coords[0].h(apply_sym(x, 32)));
    return result ?: !cube_eq(x, new_cube());
}

static void optimal(cube x, int *path, int *length)
{
    init_coords(ko_coords, 1);

    *length = 0;
    solve(x, path, length, h3, 0);
    print_moves(path, *length);
    printf(" // %d move%s\n", *length, *length==1?"":"s");
}
