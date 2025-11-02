#include "common.h"
#include "cube-table.h"

#include "coord.h"
#include "cube.h"
#include "moves.h"
#include "util.h"
#include "table.h"

#include "coord.c"
#include "cube.c"
#include "moves.c"
#include "util.c"
#include "table.c"

static FILE *fp;

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

static void write_cube_array(cube *x, int length, char *name, char *(*format)(int))
{
    fprintf(fp,
            "static cube %s[] =\n"
            "{\n"
            "    //      ", name);
    for (int i=0; i<20; ++i)
        fprintf(fp, " %3s", cubie_str[i]);
    fprintf(fp, "\n");
    for (int i=0; i<length; ++i)
    {
        char *s = format(i);
        fprintf(fp, "    [%s]%s = {{", s, strlen(s)==1?" ":"");
        for (int j=0; j<20; ++j)
            fprintf(fp, "%s%3d", j?",":"", x[i].cubies[j]);
        fprintf(fp, "}},\n");
    }
    fprintf(fp, "};\n");
}

static void write_array(int *x, int length, char *name, char *(*format)(int))
{
    fprintf(fp,
            "static int %s[] =\n"
            "{\n", name);
    for (int i=0; i<length; ++i)
    {
        char *s = format(i);
        fprintf(fp, "    [%s]%s = %d,\n", s, strlen(s)==1?" ":"", x[i]);
    }
    fprintf(fp, "};\n");
}

int main(void)
{
    //              0   1   2   3   4   5   6   7  0   1   2   3   4   5   6   7   8    9  10  11
    //            URF ULB DRB DLF URB ULF DRF DLB  RF  RB  LF  LB  UF  UB  DF  DB  UR  UL  DR  DL
    cube u    = {{  4,  5,  2,  3,  1,  0,  6,  7,  0,  1,  2,  3,  8,  9,  6,  7,  5,  4, 10, 11}};
    cube urf3 = {{ 16, 19, 17, 18, 37, 38, 36, 39, 24, 25, 26, 27,  0,  2,  1,  3, 20, 22, 21, 23}};
    cube f2   = {{  3,  2,  1,  0,  7,  6,  5,  4,  2,  3,  0,  1,  6,  7,  4,  5, 11, 10,  9,  8}};
    cube u4   = {{  4,  5,  7,  6,  1,  0,  2,  3, 17, 19, 16, 18,  8,  9, 10, 11,  5,  4,  7,  6}};
    cube rl2  = {{  5,  4,  7,  6,  1,  0,  3,  2,  2,  3,  0,  1,  4,  5,  6,  7,  9,  8, 11, 10}};
    cube urf3_i = compose(urf3, urf3);

    move_table[0] = u;
    move_table[3] = compose_3(f2, u, f2);
    for (int i=1;  i<3;  ++i) move_table[i] = compose_3(urf3_i, move_table[i-1], urf3);
    for (int i=4;  i<6;  ++i) move_table[i] = compose_3(urf3_i, move_table[i-1], urf3);
    for (int i=6;  i<12; ++i) move_table[i] = compose(move_table[i-6], move_table[i-6]);
    for (int i=12; i<18; ++i) move_table[i] = compose(move_table[i-6], move_table[i-12]);

    sym_table[0] = new_cube();
    sym_table[1] = rl2;
    for (int i=2;  i<4;  ++i) sym_table[i] = compose(sym_table[i-2],  f2);
    for (int i=4;  i<16; ++i) sym_table[i] = compose(sym_table[i-4],  u4);
    for (int i=16; i<48; ++i) sym_table[i] = compose(sym_table[i-16], urf3);

    for (int i=0; i<48; ++i)
        for (int j=i; j<48; ++j)
            if (cube_eq(compose(sym_table[i],
                                j&1 ? invert_co(sym_table[j]) : sym_table[j]),
                        new_cube()))
            {
                inv_sym_table[i] = sym_table[j];
                inv_sym_table[j] = sym_table[i];
                inv_sym[i] = j;
                inv_sym[j] = i;
                break;
            }

    fp = fopen("cube-table.c", "w");
    if (!fp) exit(1);
    write_cube_array(move_table, 18, "move_table", to_move_str);
    fprintf(fp, "\n");
    write_cube_array(sym_table, 48, "sym_table", to_str);
    fprintf(fp, "\n");
    write_cube_array(inv_sym_table, 48, "inv_sym_table", to_str);
    fprintf(fp, "\n");
    write_array(inv_sym, 48, "inv_sym", to_str);
    fclose(fp);

    return 0;
}
