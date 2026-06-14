#include "common.h"

#define NUM_CORNERS 8
#define NUM_EDGES 12
#define NUM_CUBIES 20

#define CUBE(...) (cube_t){{__VA_ARGS__}}

typedef union
{
    char cubies[20];
    struct
    {
        char corners[8];
        char edges[12];
    };
} cube_t;

FILE *fp;
long long choose[NUM_EDGES+1][NUM_EDGES+1];
long long pick[NUM_EDGES+1][NUM_EDGES+1];

static cube_t new_cube(void)
{
    return CUBE(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}

static int cube_eq(cube_t x, cube_t y)
{
    return memcmp(&x, &y, sizeof(cube_t)) == 0;
}

static cube_t mirrored_compose(cube_t x, cube_t y, int mirror)
{
    cube_t r;
    for (int i=0; i<NUM_CORNERS; i++)
    {
        r.corners[i] = x.corners[y.corners[i]&0x0f];
        if (mirror)
        {
            r.corners[i] += 0x30;
            r.corners[i] -= y.corners[i]&0xf0;
            r.corners[i] %= 0x30;
        }
        else
        {
            r.corners[i] += y.corners[i]&0xf0;
            r.corners[i] %= 0x30;
        }
    }
    for (int i=0; i<NUM_EDGES; i++)
    {
        r.edges[i] = x.edges[y.edges[i]&0x0f];
        if (mirror)
        {
            r.edges[i] += 0x20;
            r.edges[i] -= y.edges[i]&0xf0;
            r.edges[i] %= 0x20;
        }
        else
        {
            r.edges[i] += y.edges[i]&0xf0;
            r.edges[i] %= 0x20;
        }
    }
    return r;
}

static cube_t compose(cube_t x, cube_t y)
{
    return mirrored_compose(x, y, 0);
}

static cube_t compose_3(cube_t x, cube_t y, cube_t z)
{
    return compose(compose(x, y), z);
}

static cube_t invert_co(cube_t x)
{
    cube_t r;
    for (int i=0; i<NUM_CORNERS; i++)
    {
        int o = x.corners[i]&0xf0;
        r.corners[i] = x.corners[i]-o+(0x30-o)%0x30;
    }
    for (int i=0; i<NUM_EDGES; i++)
        r.edges[i] = x.edges[i];
    return r;
}

static cube_t apply_sym(cube_t x, int s, cube_t *sym_table, int *inv_sym)
{
    int mirror = s%2;
    return mirrored_compose(mirrored_compose(sym_table[inv_sym[s]], x, mirror), sym_table[s], mirror);
}

///////////////////////////////////////////////////////////////////////////////

static void write_bits(int x, int n, int reverse)
{
    for (int i=0; i<n; i++)
        fputc("01"[x>>(reverse ? i : n-1-i)&1], fp);
}

static char *to_str(int x)
{
    static char buf[256];
    snprintf(buf, 256, "%d", x);
    return buf;
}

static char *to_move_str(int x)
{
    static char buf[256];
    buf[0] = "URFDLB"[x%6];
    buf[1] = x/6 ? '1'+x/6 : 0;
    buf[2] = 0;
    return buf;
}

static void write_cases(cube_t *a, int n, char *indent, char *fmt(int))
{
    for (int i=0; i<n; i++)
    {
        char *s = fmt(i);
        fprintf(fp, "%sCASE(%s,%s", indent, s, strlen(s)==1?" ":"");
        for (int j=0; j<NUM_CUBIES; j++)
            fprintf(fp, "%s %2d", j?",":"", a[i].cubies[j]);
        fprintf(fp, ");\n");
    }
}

static void write_1d_array(long long *a, int n, char *name)
{
    fprintf(fp, "static const long long %s[] = {", name);
    for (int i=0; i<n; i++)
        fprintf(fp, "%s%lld", i?", ":"", a[i]);
    fprintf(fp, "};\n");
}

static void write_2d_array(long long *a, int n, int k, char *name)
{
    int widths[k];
    for (int j=0; j<k; j++)
    {
        int max = 0;
        char buf[13];
        for (int i=0; i<n; i++)
        {
            snprintf(buf, LENGTH(buf), "%lld", a[i*k+j]);
            max = MAX(max, strlen(buf));
        }
        widths[j] = max;
    }
    fprintf(fp,
            "\n"
            "static const long long %s[][%d] =\n"
            "{\n",
            name, k);
    for (int i=0; i<n; i++)
    {
        fprintf(fp, "    {");
        for (int j=0; j<k; j++)
            fprintf(fp, "%s%*lld", j?", ":"", widths[j], a[i*k+j]);
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
}

static void write_rank(int *unrank, int n, char mode, int k, int bits)
{
    // TODO correct type
    // TODO merge code for each array

    long long (*f)[NUM_EDGES+1] = mode=='P' ? pick : choose;
    int reverse = mode=='C';
    fprintf(fp,
           "\n"
           "static int rank_%d%c%d[] =\n"
           "{\n",
           n, mode, k);
    for (int i=0; i<f[n][k]; i++)
    {
        fprintf(fp, "    [0b");
        write_bits(unrank[i], bits, reverse);
        fprintf(fp, "] = %d,\n", i);
    }
    fprintf(fp,
            "};\n"
            "\n"
            "static int unrank_%d%c%d[] =\n"
            "{\n",
            n, mode, k);
    for (int i=0; i<f[n][k]; i++)
    {
        fprintf(fp, "    0b");
        write_bits(unrank[i], bits, reverse);
        fprintf(fp, ",\n");
    }
    fprintf(fp, "};\n");
}

int main(void)
{
    // TODO create function to write the comment string
    //                   0   1   2   3   4   5   6   7  0   1   2   3   4   5   6   7   8    9  10  11
    //                 URF ULB DRB DLF URB ULF DRF DLB  RF  RB  LF  LB  UF  UB  DF  DB  UR  UL  DR  DL
    cube_t u    = CUBE(  4,  5,  2,  3,  1,  0,  6,  7,  0,  1,  2,  3,  8,  9,  6,  7,  5,  4, 10, 11);
    cube_t rl2  = CUBE(  5,  4,  7,  6,  1,  0,  3,  2,  2,  3,  0,  1,  4,  5,  6,  7,  9,  8, 11, 10);
    cube_t f2   = CUBE(  3,  2,  1,  0,  7,  6,  5,  4,  2,  3,  0,  1,  6,  7,  4,  5, 11, 10,  9,  8);
    cube_t u4   = CUBE(  4,  5,  7,  6,  1,  0,  2,  3, 17, 19, 16, 18,  8,  9, 10, 11,  5,  4,  7,  6);
    cube_t urf3 = CUBE( 16, 19, 17, 18, 37, 38, 36, 39, 24, 25, 26, 27,  0,  2,  1,  3, 20, 22, 21, 23);
    cube_t urf3_i = compose(urf3, urf3);

    fp = fopen("data.c", "w");

    // move table
    // TODO make a function for this and the sym cubes

    cube_t move_table[18];
    move_table[0] = u;
    move_table[3] = compose_3(f2, u, f2);
    for (int i=1; i<3; ++i)
        move_table[i] = compose_3(urf3_i, move_table[i-1], urf3);
    for (int i=4; i<6; ++i)
        move_table[i] = compose_3(urf3_i, move_table[i-1], urf3);
    for (int i=6; i<12; ++i)
        move_table[i] = compose(move_table[i-6], move_table[i-6]);
    for (int i=12; i<18; ++i)
        move_table[i] = compose(move_table[i-6], move_table[i-12]);
    fprintf(fp,
            "static cube_t get_move_cube(int x)\n"
            "{\n"
            "    switch (x)\n"
            "    {\n"
            "#define CASE(x, ...) case x: return CUBE(__VA_ARGS__);\n"
            "        //      URF ULB DRB DLF URB ULF DRF DLB  RF  RB  LF  LB  UF  UB  DF  DB  UR  UL  DR  DL\n");
    write_cases(move_table, LENGTH(move_table), "        ", to_move_str);
    fprintf(fp,
            "        default: UNREACHABLE();\n"
            "    }\n"
            "}\n"
            "\n");

    // move & sym cubes

    cube_t sym_table[48];
    sym_table[0] = new_cube();
    sym_table[1] = rl2;
    for (int i=2; i<4; ++i)
        sym_table[i] = compose(f2, sym_table[i-2]);
    for (int i=4; i<16; ++i)
        sym_table[i] = compose(u4, sym_table[i-4]);
    for (int i=16; i<48; ++i)
        sym_table[i] = compose(urf3, sym_table[i-16]);
    fprintf(fp,
            "static cube_t get_sym_cube(int x)\n"
            "{\n"
            "    switch (x)\n"
            "    {\n"
            "        //      URF ULB DRB DLF URB ULF DRF DLB  RF  RB  LF  LB  UF  UB  DF  DB  UR  UL  DR  DL\n");
    write_cases(sym_table, LENGTH(sym_table), "        ", to_str);
    fprintf(fp, "#undef CASE\n"
            "        default: UNREACHABLE();\n"
            "    }\n"
            "}\n");

    fp = freopen("data.h", "w", fp);
    fprintf(fp,
            "#ifndef DATA_H\n"
            "#define DATA_H\n"
            "\n"
            "#include \"moves.h\"\n"
            "\n"
            "static cube_t get_move_cube(int);\n"
            "static cube_t get_sym_cube(int);\n"
            "\n");

    // sym inverses

    int inv_sym[48];
    for (int i=0; i<48; ++i)
        for (int j=i; j<48; j+=2)
            if (cube_eq(compose(sym_table[i], i&1 ? invert_co(sym_table[j]) : sym_table[j]), new_cube()))
            {
                inv_sym[i] = j;
                inv_sym[j] = i;
                break;
            }
    fprintf(fp,
            "static int inv_sym[] =\n"
            "{\n");
    for (int k=0; k<2; k++)
        for (int i=0; i<3; i++)
        {
            fprintf(fp, "    ");
            for (int j=0; j<16; j++)
                fprintf(fp, "%2d, ", inv_sym[i*16+j]+k*48);
            fprintf(fp, "\n");
        }
    fprintf(fp,
            "};\n"
            "\n");

    // symmetries of moves

    int sym_moves[48][18];
    fprintf(fp,
            "static int sym_moves[48][18] =\n"
            "{\n");
    for (int i=0; i<48; i++)
    {
        fprintf(fp, "    [%d] =%s{", i, i<10?"  ":" ");
        for (int j=0; j<18; j++)
        {
            if (j)
                fprintf(fp, ", ");
            cube_t x = apply_sym(move_table[j], i, sym_table, inv_sym);
            for (int k=0; k<18; k++)
                if (cube_eq(x, move_table[k]))
                {
                    fprintf(fp, "%2s", to_move_str(k));
                    goto next;
                }
            fprintf(fp, "xx");
            cube_t a[] = {move_table[j], x, move_table[16]};
            write_cases(a, LENGTH(a), "\n", to_str);
            exit(1);
next:
        }
        fprintf(fp, "},\n");
    }
    fprintf(fp,
            "};\n"
            "\n");

    // combinatorics

    long long pow2[NUM_EDGES];
    for (int i=0; i<LENGTH(pow2); i++)
        pow2[i] = 1<<i;
    write_1d_array(pow2, LENGTH(pow2), "pow2");

    long long pow3[NUM_CORNERS];
    for (int i=0, x=1; i<LENGTH(pow3); i++, x*=3)
        pow3[i] = x;
    write_1d_array(pow3, LENGTH(pow3), "pow3");

    long long fact[NUM_EDGES+1];
    for (long long i=0, x=1; i<LENGTH(fact); i++, x*=i)
        fact[i] = x;
    write_1d_array(fact, LENGTH(fact), "fact");

    for (int i=0; i<NUM_EDGES+1; i++)
        pick[i][0] = 1;
    for (int i=1; i<NUM_EDGES+1; i++)
        for (int j=1; j<NUM_EDGES+1; j++)
            pick[i][j] = i*pick[i-1][j-1];
    write_2d_array((long long *)pick, NUM_EDGES+1, NUM_EDGES+1, "pick");

    for (int i=0; i<NUM_EDGES+1; i++)
        choose[i][0] = 1;
    for (int i=1; i<NUM_EDGES+1; i++)
        for (int j=1; j<NUM_EDGES+1; j++)
            choose[i][j] = choose[i-1][j-1]+choose[i-1][j];
    write_2d_array((long long *)choose, NUM_EDGES+1, NUM_EDGES+1, "choose");

    // ranking

    void write_rank_C(int n)
    {
        int a[choose[n][4]];
        for (int i=0, x=0; i<choose[n][4]; x++)
            if (__builtin_popcount(x) == 4)
                a[i++] = x;
        write_rank(a, n, 'C', 4, n);
    }

    write_rank_C(8);
    write_rank_C(12);

    int unrank_4P4[pick[4][4]];

    void generate_perms(int n)
    {
        static int perm[4];
        static int v[4];

        int parity(void)
        {
            int r = 0;
            for (int i=0; i<n; i++)
                for (int j=i+1; j<n; j++)
                    r ^= perm[i]>perm[j];
            return r;
        }

        if (n==4)
        {
            static int indices[2] = {0, 1};
            int bits = 0;
            for (int i=0; i<n; i++)
                bits |= perm[i]<<(2*i);
            int *i = &indices[parity()];
            unrank_4P4[*i] = bits;
            *i += 2;
            return;
        }
        for (int i=0; i<4; i++)
            if (!v[i])
            {
                v[i] = 1;
                perm[n] = i;
                generate_perms(n+1);
                v[i] = 0;
            }
    }

    generate_perms(0);
    write_rank(unrank_4P4, 4, 'P', 4, 8);

    fprintf(fp,
            "\n"
            "#endif\n");
    fclose(fp);
    return 0;
}