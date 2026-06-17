void run_tests(void)
{
    char *name;
    
    void check(int x, int y)
    {
        if (x != y)
        {
            fprintf(stderr, "test '%s' failed (%d != %d)\n", name, x, y);
            exit(1);
        }
    }

    void test(char *s)
    {
        name = s;
        printf("running test '%s'\n", name);
    }

    void test_coord(char *name, long long get(cube_t), void set(cube_t *, long long), long long max)
    {
        test(name);
        for (long long i=0; i<max; ++i)
        {
            cube_t x = new_cube();
            set(&x, i);
            check(i, get(x));
        }
    }

    test("moves");
    {
        ASSERT(LENGTH(scrambles) == LENGTH(solutions));
        for (int i=0; i<LENGTH(scrambles); ++i)
        {
            cube_t x = apply_moves(new_cube(), scrambles[i], 9);
            cube_t y = apply_moves(new_cube(), solutions[i], 9);
            cube_t z = compose(x, y);
            check(cube_eq(z, new_cube()), 1);
        }
    }

    test("inverse");
    {
        int moves[256];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            cube_t y = inverse(x);
            cube_t z = compose(x, y);
            check(cube_eq(z, new_cube()), 1);
        }
    }

    test("syms");
    {
        int moves[256];
        int transformed_moves[LENGTH(moves)];
        for (int length=0; length<LENGTH(moves); ++length)
        {
            make_scramble(moves, length);
            cube_t x = apply_moves(new_cube(), moves, length);
            for (int s=0; s<NUM_SYMS; ++s)
            {
                for (int i=0; i<length; ++i)
                    transformed_moves[i] = sym_moves[s%48][moves[i]];
                if (s>47)
                    inverse_moves(transformed_moves, length);
                check(cube_eq(apply_sym(x, s), apply_moves(new_cube(), transformed_moves, length)), 1);
            }
        }
    }

    test_coord("eo", get_eo, set_eo, EO_MAX);
    test_coord("co", get_co, set_co, CO_MAX);
    test_coord("csep", get_csep, set_csep, CSEP_MAX);
    test_coord("esep", get_esep, set_esep, ESEP_MAX);
    test_coord("orbit_fast", get_orbit_fast, set_orbit_fast, ORBIT_MAX);

    test("solver");
    {
        for (int i=0; i<LENGTH(scrambles); ++i)
        {
            int moves[256], length;
            cube_t x = apply_moves(new_cube(), scrambles[i], 9);
            optimal(x, moves, &length);
            check(length<=9, 1);
            x = apply_moves(x, moves, length);
            check(cube_eq(x, new_cube()), 1);
        }
    }

    printf("all tests passed\n");
}