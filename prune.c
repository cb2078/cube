static void print_completion(long long i, long long n)
{
    fprintf(stderr, "\rcompletion=%.2f%%", (double)i/n*100);
}

static void clear_stderr(void)
{
    fprintf(stderr, "\r");
    for (int i=0; i<80; ++i)
        fprintf(stderr, " ");
    fprintf(stderr, "\r");
}

static void init_sym(struct coord *c)
{
    if (!c->num_syms)
        return;

    // TODO put this in its own function
    struct coord *b = c->base;
    ASSERT(b->max);
    for (long long i=0; i<b->max; ++i)
    {
        cube_t x = b->set(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = b->get(y);
            c->self_syms[i] |= (i==k)<<s;
        }
        print_completion(i, b->max);
    }
    clear_stderr();

    int class = 0;
    memset(c->to_class, 0xff, sizeof(c->to_class[0])*b->max);
    for (long long i=0; i<b->max; ++i)
    {
        if (c->to_class[i] != -1)
            continue;
        cube_t x = b->set(i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = b->get(y);
            c->to_class[k] = class;
            c->to_sym[k] = inv_sym[s];
        }
        c->to_rep[class++] = i;
        print_completion(i, b->max);
    }
    clear_stderr();
    ASSERT(class == c->classes);
}

static void init_prune_table(struct coord *c)
{
    void print(long long n, int depth, int backsearch)
    {
        print_completion(n, c->max);
        fprintf(stderr, "depth=%d", depth);
        if (backsearch)
            fprintf(stderr, " (backsearch)");
    }

    if (table_read(c->table = table_new(c->max, 4, c->name)))
        return;
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, c->get(new_cube()), 0);
    for (int depth=1, t=0, backsearch=0; c->table->count<c->max && depth<c->table->mask; ++depth)
    {
        long long m = c->table->count;
        for (long long i=0; i<c->max; ++i)
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
                    cube_t x = apply_move(c->set(i), moves[j]);
                    long long k = c->get(x);
                    if (!backsearch && table_get(c->table, k) == c->table->mask)
                    {
                        table_set(c->table, k, depth);
                        if (!c->num_syms)
                            continue;
                        struct coord *b = c->base;
                        for (int s=1; s<c->num_syms; ++s)
                        {
                            if (~c->self_syms[b->get(x)]>>s&1)
                                continue;
                            cube_t y = apply_sym(x, s);
                            long long l = c->get(y);
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
            if (c->table->count*10000/c->max > t)
                print(c->table->count, depth, backsearch), ++t;
        }
        backsearch = c->table->count>c->max/2;
        print(c->table->count, depth, backsearch);
        clear_stderr();
        LOG("%s[%d] = %lld\n", depth<10?" ":"", depth, c->table->count-m);
    }
    clear_stderr();
    if (c->table->count!=c->max)
        LOG("skpped %lld entries\n", c->max-c->table->count);
    table_write(c->table);
}

static void init_tetrad_twist_table(void)
{
    cube_t separate_corners(cube_t x)
    {
        cube_t result = x;
        int j=0, k=0;
        for (int i=0; i<NUM_CORNERS; ++i)
            if (x.corners[i]<4)
                result.urf_tetrad[j++] = x.corners[i];
            else
                result.urb_tetrad[k++] = x.corners[i];
        assert(j==4 && k==4);
        return result;
    }

    int n = fact[8];
    tetrad_twist_table = table_new(n, 4, "tetrad-twist");
    if (table_read(tetrad_twist_table)) return;

    for (int i=0; i<n; ++i)
    {
        cube_t x = new_cube();
        set_permutation(x.corners, NUM_CORNERS, i);
        x = separate_corners(x);
        int moves[64], length;
        solve(x, moves, &length, h_cp5, HTR_MASK);
        x = apply_moves(x, moves, length);
        table_set(tetrad_twist_table, i, get_permutation(x.corners, 3));
        print_completion(i, n);
    }
    clear_stderr();

    table_write(tetrad_twist_table); // todo error handling
}
