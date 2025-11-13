static void init_prune_table(struct coord *c)
{
    void print(long long n, int depth, int backsearch)
    {
        fprintf(stderr, "\rdepth=%d comletion=%.2f%%", depth, (double)n/c->max*100);
        if (backsearch)
            fprintf(stderr, " (backsearch)");
    }

    void clear(void)
    {
        fprintf(stderr, "\r                                           \r");
    }

    if (table_read(c->table = table_new(c->max, 4, c->name)))
        return;
    memset(c->table->data, 0xff, c->table->size);
    table_set(c->table, coord_get(c, new_cube()), 0);
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
                    cube_t x = apply_move(coord_set(c, i), moves[j]);
                    long long k = coord_get(c, x);
                    if (!backsearch && table_get(c->table, k) == c->table->mask)
                    {
                        table_set(c->table, k, depth);
                        if (c->type!=SYM_COMP)
                            continue;
                        struct coord *b = &c->coords[0];
                        for (int s=1; s<c->num_syms; ++s)
                        {
                            if (~c->self_syms[coord_get(b, x)]>>s&1)
                                continue;
                            cube_t y = apply_sym(x, s);
                            int l = coord_get(c, y);
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
        clear();
        printf("[%d]%s= %lld\n", depth, depth<10?"  ":" ", c->table->count-m);
    }
    clear();
    if (c->table->count!=c->max)
        printf("skpped %lld entries\n", c->max-c->table->count);
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
        fprintf(stderr, "\rcompletion=%.2f%%", (float)i/n*100);
    }
    fprintf(stderr, "\r                                        \r");

    table_write(tetrad_twist_table); // todo error handling
}
