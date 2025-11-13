#include "common.h"
#include "data.h"

#include "cube.h"
#include "moves.h"
#include "util.h"
#include "table.h"

#include "cube.c"
#include "moves.c"
#include "util.c"
#include "table.c"

static FILE *fp;

static int transform_move(int move, int sym)
{
    int orders[4] = {16, 4, 2, 1};
    for (int i=0; i<4; ++i)
        for (; sym>=orders[i]; sym-=orders[i])
            switch (i)
            {
                case 0:
                    move = make_move(move_face(move)/3*3+move_axis(move+1), move_amount(move));
                    break;
                case 1:
                    move = make_move(move_face(move_axis(move)+move), move_amount(move));
                    break;
                case 2:
                    move = make_move(move_face(move_axis(move)==F ? move : move+3), move_amount(move));
                    break;
                case 3:
                    move = make_move(move_face(move_axis(move)==R ? 3+move : move), 4-move_amount(move));
                    break;
                default:
                    UNREACHABLE();
            }
    return move;
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

static void write_cube_array(cube_t *x, int length, char *name, char *(*format)(int))
{
    fprintf(fp,
            "static cube_t %s[] =\n"
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

static void write_move_array(void)
{
    fprintf(fp,
            "static int sym_moves[48][18] =\n"
            "{\n");
    for (int i=0; i<48; ++i)
    {
        char *s = to_str(i);
        fprintf(fp, "    [%s]%s = {", s, strlen(s)==1?" ":"");
        for (int j=0; j<18; ++j)
            fprintf(fp, "%s%3s", j?",":"", to_move_str(sym_moves[i][j]));
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
}

static void write_array(int *x, int cols, int rows, char *name)
{
    fprintf(fp,
            "static int %s[] =\n"
            "{\n", name);
    for (int i=0; i<rows; ++i)
    {
        fprintf(fp, "    ");
        for (int j=0; j<cols; ++j)
            fprintf(fp, "%s%2d,", j?" ":"", x[i*cols+j]);
        fprintf(fp, "\n");
    }
    fprintf(fp, "};\n");
}

int main(void)
{
    //              0   1   2   3   4   5   6   7  0   1   2   3   4   5   6   7   8    9  10  11
    //            URF ULB DRB DLF URB ULF DRF DLB  RF  RB  LF  LB  UF  UB  DF  DB  UR  UL  DR  DL
    cube_t u    = {{  4,  5,  2,  3,  1,  0,  6,  7,  0,  1,  2,  3,  8,  9,  6,  7,  5,  4, 10, 11}};
    cube_t rl2  = {{  5,  4,  7,  6,  1,  0,  3,  2,  2,  3,  0,  1,  4,  5,  6,  7,  9,  8, 11, 10}};
    cube_t f2   = {{  3,  2,  1,  0,  7,  6,  5,  4,  2,  3,  0,  1,  6,  7,  4,  5, 11, 10,  9,  8}};
    cube_t u4   = {{  4,  5,  7,  6,  1,  0,  2,  3, 17, 19, 16, 18,  8,  9, 10, 11,  5,  4,  7,  6}};
    cube_t urf3 = {{ 16, 19, 17, 18, 37, 38, 36, 39, 24, 25, 26, 27,  0,  2,  1,  3, 20, 22, 21, 23}};
    cube_t urf3_i = compose(urf3, urf3);

    move_cubes[0] = u;
    move_cubes[3] = compose_3(f2, u, f2);
    for (int i=1;  i<3;  ++i) move_cubes[i] = compose_3(urf3_i, move_cubes[i-1], urf3);
    for (int i=4;  i<6;  ++i) move_cubes[i] = compose_3(urf3_i, move_cubes[i-1], urf3);
    for (int i=6;  i<12; ++i) move_cubes[i] = compose(move_cubes[i-6], move_cubes[i-6]);
    for (int i=12; i<18; ++i) move_cubes[i] = compose(move_cubes[i-6], move_cubes[i-12]);

    sym_cubes[0] = new_cube();
    sym_cubes[1] = rl2;
    for (int i=2;  i<4;  ++i) sym_cubes[i] = compose(f2,   sym_cubes[i-2]);
    for (int i=4;  i<16; ++i) sym_cubes[i] = compose(u4,   sym_cubes[i-4]);
    for (int i=16; i<48; ++i) sym_cubes[i] = compose(urf3, sym_cubes[i-16]);

    for (int i=0; i<18; ++i)
        for (int j=0; j<48; ++j)
            sym_moves[j][i] = transform_move(i, j);

    for (int i=0; i<48; ++i)
        for (int j=i; j<48; j+=2)
            if (cube_eq(compose(sym_cubes[i], i&1 ? invert_co(sym_cubes[j]) : sym_cubes[j]), new_cube()))
            {
                inv_sym[i] = j;
                inv_sym[j] = i;
                break;
            }

    fp = fopen("data.c", "w");
    if (!fp) exit(1);
    write_cube_array(move_cubes, 18, "move_cubes", to_move_str);
    fprintf(fp, "\n");
    write_move_array();
    fprintf(fp, "\n");
    write_cube_array(sym_cubes, 48, "sym_cubes", to_str);
    fprintf(fp, "\n");
    write_array(inv_sym, 16, 3, "inv_sym");
    fclose(fp);

    return 0;
}
