#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define unreachable() assert(0 && "unreachable")
#define length(x) (sizeof(x)/sizeof(x[0]))

static FILE *fp;

#define NUM_CORNERS 8
#define NUM_EDGES 12

enum mode
{
    GET,
    SET,
};

struct coord
{
    enum
    {
        RAW,
        SYM,
        COMPOSITE,
    } type;
    union
    {
        struct
        {
            enum
            {
                COMBINATION,
                ORIENTATION,
                PERMUTATION,
                PARTIAL_PERMUTATION,
                TETRAD_TWIST,
            } indexer;
            enum
            {
                CORNERS,
                EDGES,
                TETRAD_URF,
                TETRAD_URB,
                SLICE_UD,
                SLICE_RL,
                SLICE_FB,
            } subset;
            int k;
        };
        struct
        {
            struct coord *ref;
            int symmetries[4];
        };
        struct
        {
            int count;
            struct coord *coords;
        };
    };
};

struct cubie_subset
{
    char *name;
    int start, end, length;
};

static const struct coord coord_eo = {
    .type = RAW,
    .indexer = ORIENTATION,
    .subset = EDGES,
};

static const struct coord coord_co = {
    .type = RAW,
    .indexer = ORIENTATION,
    .subset = CORNERS,
};

static const struct coord coord_ud_slice = {
    .type = RAW,
    .indexer = COMBINATION,
    .subset = SLICE_UD,
};

static const struct coord tw_coords[4] = {
    [0] = coord_eo,
    [1] = {
        .type = COMPOSITE,
        .count = 2,
        .coords = (struct coord[]){
            [0] = coord_co,
            [1] = coord_ud_slice,
        },
    },
    [2] = {
        .type = COMPOSITE,
        .count = 3,
        .coords = (struct coord[]){
            [0] = {
                .type = RAW,
                .indexer = COMBINATION,
                .subset = SLICE_RL,
            },
            [1] = {
                .type = RAW,
                .indexer = COMBINATION,
                .subset = TETRAD_URF,
            },
            [2] = {
                .type = RAW,
                .indexer = TETRAD_TWIST,
                .subset = TETRAD_URF,
            },
        },
    },
    [3] = {
        .type = COMPOSITE,
        .count = 5,
        .coords = (struct coord[]){
            [0] = {
                .type = RAW,
                .indexer = PERMUTATION,
                .subset = TETRAD_URF,
            },
            [1] = {
                .type = RAW,
                .indexer = PERMUTATION,
                .subset = SLICE_UD,
            },
            [2] = {
                .type = RAW,
                .indexer = PERMUTATION,
                .subset = SLICE_RL,
            },
            [3] = {
                .type = RAW,
                .indexer = PARTIAL_PERMUTATION,
                .subset = SLICE_FB,
                .k = 2,
            },
            [4] = {
                .type = RAW,
                .indexer = PARTIAL_PERMUTATION,
                .subset = TETRAD_URB,
                .k = 1,
            },
        },
    },
};

static const struct cubie_subset subsets[] = {
    [CORNERS] = {
        .name = "corners",
        .start = 0,
        .end = NUM_CORNERS,
        .length = NUM_CORNERS,
    },
    [EDGES] = {
        .name = "edges",
        .start = 0,
        .end = NUM_EDGES,
        .length = NUM_EDGES,
    },
    [TETRAD_URF] = {
        .name = "tetrads[TETRAD_URF]",
        .start = 0,
        .end = NUM_CORNERS,
        .length = 4,
    },
    [TETRAD_URB] = {
        .name = "tetrads[TETRAD_URB]",
        .start = 4,
        .end = NUM_CORNERS,
        .length = 4,
    },
    [SLICE_UD] = {
        .name = "slices[SLICE_UD]",
        .start = 0,
        .end = NUM_EDGES,
        .length = 4,
    },
    [SLICE_RL] = {
        .name = "slices[SLICE_RL]",
        .start = 4,
        .end = NUM_EDGES,
        .length = 4,
    },
    [SLICE_FB] = {
        .name = "slices[SLICE_FB]",
        .start = 8,
        .end = NUM_EDGES,
        .length = 4,
    },
};

static int write_coord_rec(const struct coord *x, char *s, enum mode mode, int remaining)
{
#define S subsets[x->subset]

    void write_offset(char op)
    {
        if (!S.start)
            return;
        int n = x->indexer==PERMUTATION ? S.length : S.end-S.start;
        fprintf(fp, "    for (int i=0; i<%d; ++i) x.%s[i] %c= %d;\n", n, S.name, op, S.start);
    }

    void write_get_offset(void)
    {
        if (mode == GET)
            write_offset('-');
    }

    void write_set_offset(void)
    {
        if (mode == SET)
            write_offset('+');
    }

    int max = 1;
    switch (x->type)
    {
        case RAW:
            char amount[256], expr[256];
            write_get_offset();
            switch (x->indexer)
            {
                case COMBINATION:
                    sprintf(expr, "combination(x.%s, %d, %d", S.name, S.end-S.start, S.length);
                    sprintf(amount, "choose(%d, %d)", S.end-S.start, S.length);
                    max = choose(S.end-S.start, S.length);
                    break;
                case ORIENTATION:
                    switch (x->subset)
                    {
                        case CORNERS:
                            sprintf(expr, "co(%s", mode==GET ? "x" : "&x");
                            sprintf(amount, "pow3[NUM_CORNERS-1]");
                            max = pow3[NUM_CORNERS-1];
                            break;
                        case EDGES:
                            sprintf(expr, "eo(%s", mode==GET ? "x" : "&x");
                            sprintf(amount, "pow2[NUM_EDGES-1]");
                            max = pow2[NUM_EDGES-1];
                            break;
                        default:
                            unreachable();
                    }
                    break;
                case PERMUTATION:
                    sprintf(expr, "permutation(x.%s, %d", S.name, S.length);
                    sprintf(amount, "fact[%d]", S.length);
                    max = fact[S.length];
                    break;
                case PARTIAL_PERMUTATION:
                    sprintf(expr, "partial_permutation(x.%s, %d, %d", S.name, S.length, x->k);
                    sprintf(amount, "pick(%d, %d)", S.length, x->k);
                    max = pick(S.length, x->k);
                    break;
                case TETRAD_TWIST:
                    sprintf(expr, "tetrad_twist(%s", mode==GET ? "x" : "&x");
                    sprintf(amount, "6");
                    max = 6;
                    break;
                default:
                    unreachable();
            }
            fprintf(fp, "    ");
            if (mode == GET)
                fprintf(fp, "result += get_");
            else
                fprintf(fp, "i = result%%%s;\n    set_", amount);
            fprintf(fp, "%s", expr);
            if (mode == SET)
                fprintf(fp, ", i");
            fprintf(fp, ")%s;\n", s);
            write_set_offset();
            if (remaining && mode == SET)
                fprintf(fp, "    result /= %s;\n", amount);
            else
                strcat(s, " * "), strcat(s, amount);
            break;
        case SYM:
            exit(1);
        case COMPOSITE:
            for (int i=0; i<x->count; ++i)
                max *= write_coord_rec(&x->coords[i], s, mode, x->count-1-i);
            break;
    }
    return max;
#undef S
}

int main(void)
{
    char buf[256];
    fp = fopen("coord.c", "w");
    int n = length(tw_coords);
    int max[4];

    fprintf(fp,
            "#include \"coord.h\"\n"
            "#include \"table.h\"\n"
            "#include \"util.h\"\n");

    for (int i=0; i<n; ++i)
    {
        fprintf(fp,
                "\n"
               "static int get_tw_g%d(cube x)\n"
               "{\n"
               "    int result = 0;\n", i);
        memset(buf, 0x00, sizeof(buf));
        max[i] = write_coord_rec(tw_coords+i, buf, GET, 0);
        fprintf(fp,
                "    return result;\n"
               "}\n");
    }

    for (int i=0; i<n; ++i)
    {
        fprintf(fp,
                "\n"
               "static cube set_tw_g%d(int result)\n"
               "{\n"
               "    cube x = new_cube();\n"
               "    int i;\n", i);
        memset(buf, 0x00, sizeof(buf));
        (void)write_coord_rec(tw_coords+i, buf, SET, 0);
        fprintf(fp,
                "    return x;\n"
                "}\n");
    }

    for (int i=0; i<n; ++i)
        fprintf(fp,
                "\n"
               "static int h_tw_g%d(cube x)\n"
               "{\n"
               "    return table_get(tw_coords[%d].table, tw_coords[%d].get(x));\n"
               "}\n", i, i, i);

    fprintf(fp,
            "\n"
           "coord tw_coords[] = \n"
           "{\n"
           // "    //         U  R  F  D  L  B\n"
           );
    static const char *quater_turns[] = {
        "{1, 1, 1, 1, 1, 1}",
        "{1, 1, 0, 1, 1, 0}",
        "{1, 0, 0, 1, 0, 0}",
        "{0, 0, 0, 0, 0, 0}",
    };
    for (int i=0; i<n; ++i)
        fprintf(fp,
                "    {.name=\"tw_g%d\", .get=get_tw_g%d, .set=set_tw_g%d, .h=h_tw_g%d, .quater_turns=%s, .order=%d},\n",
                i, i, i, i, quater_turns[i], max[i]);
    fprintf(fp, "};\n");

    fclose(fp);
    fp = fopen("coord.h", "w");

    fprintf(fp,
            "#include \"cube.h\"\n"
            "\n"
            "typedef struct\n"
            "{\n"
            "    char *name;\n"
            "    int (*get)(cube);\n"
            "    cube (*set)(int);\n"
            "    int (*h)(cube);\n"
            "    int order;\n"
            "    table table;\n"
            "    int quater_turns[6];\n"
            "} coord;\n");

    fprintf(fp,
            "\n"
            "extern coord tw_coords[%d];\n", n);

    fclose(fp);

    return 0;
}
