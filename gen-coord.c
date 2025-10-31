#include "common.h"

#include "util.h"
#include "util.c"

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

static long long write_coord_rec(const struct coord *x, enum mode mode, int at_start, int at_end)
{
#define S subsets[x->subset]

    void write_offset(char op)
    {
        if (!S.start)
            return;
        int n = x->indexer==PERMUTATION ? S.length : S.end-S.start;
        fprintf(fp, "    for (int i=0; i<%d; ++i) *(x.%s+i) %c= %d;\n", n, S.name, op, S.start);
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

    long long max = 1;
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
            if (mode == SET)
            {
                fprintf(fp, "    i = result");
                if (!at_end)
                    fprintf(fp, "%%%s", amount);
                fprintf(fp, ";\n");
            }
            fprintf(fp, "    %s%s%s)%s;\n",
                    mode==GET ?  "result += get_" : "set_",
                    expr,
                    mode==SET ? ", i" : "",
                    mode==GET&&!at_start ? " * i" : "");
            write_set_offset();
            if (!at_end)
                fprintf(fp, "    %s= %s;\n", mode==GET ? "i *" : "result /", amount);
            fprintf(fp, "\n");
            break;
        case SYM:
            exit(1);
        case COMPOSITE:
            for (int i=0; i<x->count; ++i)
                max *= write_coord_rec(&x->coords[i], mode, i==0, i==x->count-1);
            break;
    }
    return max;
#undef S
}

static long long write_coord(const struct coord *x, enum mode mode)
{
    return write_coord_rec(x, mode, 1, 1);
}

int main(void)
{
    fp = fopen("coord.c", "w");
    int n = length(tw_coords);
    long long max[n];

    for (int i=0; i<n; ++i)
    {
        fprintf(fp,
                "static long long get_tw_g%d(cube x)\n"
                "{\n"
                "    long long result=0%s;\n"
                "\n", i,
                tw_coords[i].type==COMPOSITE && tw_coords[i].count>1 ? ", i=1" : "");
        max[i] = write_coord(tw_coords+i, GET);
        fprintf(fp,
                "    return result;\n"
                "}\n");
    }

    for (int i=0; i<n; ++i)
    {
        fprintf(fp,
                "\n"
               "static cube set_tw_g%d(long long result)\n"
               "{\n"
               "    cube x = new_cube();\n"
               "    long long i;\n"
               "\n", i);
        (void)write_coord(tw_coords+i, SET);
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
            "static coord tw_coords[] =\n"
            "{\n");
    static const char *quater_turns[] = {
        "{1, 1, 1, 1, 1, 1}",
        "{1, 1, 0, 1, 1, 0}",
        "{1, 0, 0, 1, 0, 0}",
        "{0, 0, 0, 0, 0, 0}",
    };
    for (int i=0; i<n; ++i)
        fprintf(fp,
                "    {.name=\"tw_g%d\", .get=get_tw_g%d, .set=set_tw_g%d, .h=h_tw_g%d, .quater_turns=%s, .order=%lld},\n",
                i, i, i, i, quater_turns[i], max[i]);
    fprintf(fp, "};\n");

    fclose(fp);
    fp = fopen("coord.h", "w");

    fprintf(fp,
            "#include \"cube.h\"\n"
            "#include \"table.h\"\n"
            "\n"
            "typedef struct\n"
            "{\n"
            "    char *name;\n"
            "    long long (*get)(cube);\n"
            "    cube (*set)(long long);\n"
            "    int (*h)(cube);\n"
            "    long long order;\n"
            "    table *table;\n"
            "    int quater_turns[6];\n"
            "} coord;\n");

    fprintf(fp,
            "\n"
            "static coord tw_coords[%d];\n", n);

    fclose(fp);

    return 0;
}
