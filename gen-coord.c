#include "common.h"

#include "util.h"
#include "util.c"

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
    char *name;
    long long order; // this will be set when writing the coordinate
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

static int num_sym_coords = 0;
static struct coord *sym_coords[256];

struct cubie_subset
{
    char *name;
    int start, end, length;
};

static struct cubie_subset subsets[] = {
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

static void write_coord_rec(struct coord *x, enum mode mode, int at_start, int at_end)
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

    x->order = 1;
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
                    x->order = choose(S.end-S.start, S.length);
                    break;
                case ORIENTATION:
                    switch (x->subset)
                    {
                        case CORNERS:
                            sprintf(expr, "co(%s", mode==GET ? "x" : "&x");
                            sprintf(amount, "pow3[NUM_CORNERS-1]");
                            x->order = pow3[NUM_CORNERS-1];
                            break;
                        case EDGES:
                            sprintf(expr, "eo(%s", mode==GET ? "x" : "&x");
                            sprintf(amount, "pow2[NUM_EDGES-1]");
                            x->order = pow2[NUM_EDGES-1];
                            break;
                        default:
                            UNREACHABLE();
                    }
                    break;
                case PERMUTATION:
                    sprintf(expr, "permutation(x.%s, %d", S.name, S.length);
                    sprintf(amount, "fact[%d]", S.length);
                    x->order = fact[S.length];
                    break;
                case PARTIAL_PERMUTATION:
                    sprintf(expr, "partial_permutation(x.%s, %d, %d", S.name, S.length, x->k);
                    sprintf(amount, "pick(%d, %d)", S.length, x->k);
                    x->order = pick(S.length, x->k);
                    break;
                case TETRAD_TWIST:
                    sprintf(expr, "tetrad_twist(%s", mode==GET ? "x" : "&x");
                    sprintf(amount, "6");
                    x->order = 6;
                    break;
                default:
                    UNREACHABLE();
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
            if (mode == SET)
            {
                fprintf(fp, "    result = tw_g0_eqv_class_to_rep[result];\n");
                fprintf(fp, "\n");
            }
            // TODO try using write_coord (without the 'ref' suffix)
            write_coord_rec(x->ref, mode, at_start, at_end);
            if (mode == GET)
            {
                if (!at_end)
                    fprintf(fp, "    long long sym = tw_g0_coord_to_rep_sym[result];\n");
                fprintf(fp, "    result = tw_g0_coord_to_eqv_class[result];\n");
                fprintf(fp, "\n");
            }
            // since this function is ran twice for get and set, only do the following once
            if (mode == GET)
                sym_coords[num_sym_coords++] = x->ref;
            break;
        case COMPOSITE:
            for (int i=0; i<x->count; ++i)
            {
                write_coord_rec(&x->coords[i], mode, i==0, i==x->count-1);
                x->order *= x->coords[i].order;
            }
            break;
    }
#undef S
}

static void write_coord(struct coord *x, enum mode mode)
{
    write_coord_rec(x, mode, 1, 1);
}

static void write_coord_getter(struct coord *x, char *name)
{
    fprintf(fp,
            "static long long get_%s(cube x)\n"
            "{\n"
            "    long long result=0%s;\n"
            "\n",
            name,
            x->type==COMPOSITE && x->count>1 ? ", i=1" : "");
    write_coord(x, GET);
    fprintf(fp,
            "    return result;\n"
            "}\n"
            "\n");
}

static void write_coord_setter(struct coord *x, char *name)
{
    fprintf(fp,
            "static cube set_%s(long long result)\n"
            "{\n"
            "    cube x = new_cube();\n"
            "    long long i;\n"
            "\n",
            name);
    write_coord(x, SET);
    fprintf(fp,
            "    return x;\n"
            "}\n"
            "\n");
}

static void write_coords(struct coord *x, int n, char *name)
{
    char buf[256];

    for (int i=0; i<n; ++i)
    {
        snprintf(buf, 256, "%s_g%d", name, i);
        write_coord_getter(x+i, buf);
    }

    for (int i=0; i<n; ++i)
    {
        snprintf(buf, 256, "%s_g%d", name, i);
        write_coord_setter(x+i, buf);
    }

    for (int i=0; i<num_sym_coords; ++i)
    {
        snprintf(buf, 256, "%s", sym_coords[i]->name);
        write_coord_getter(sym_coords[i], buf);
    }

    for (int i=0; i<num_sym_coords; ++i)
    {
        snprintf(buf, 256, "%s", sym_coords[i]->name);
        write_coord_setter(sym_coords[i], buf);
    }

    for (int i=0; i<n; ++i)
        fprintf(fp,
                "static int h_tw_g%d(cube x)\n"
                "{\n"
                "    return table_get(tw_coords[%d].table, tw_coords[%d].get(x));\n"
                "}\n"
                "\n", i, i, i);

    static char *quater_turns[] = {
        "{1, 1, 1, 1, 1, 1}",
        "{1, 1, 0, 1, 1, 0}",
        "{1, 0, 0, 1, 0, 0}",
        "{0, 0, 0, 0, 0, 0}",
    };
    fprintf(fp,
            "static coord tw_coords[] =\n"
            "{\n");
    for (int i=0, j=0; i<n; ++i)
    {
        struct coord *sym = x[i].type==SYM ? &x[i]
            : x[i].type==COMPOSITE && x[i].coords[0].type==SYM ? &x[i].coords[0]
            : 0;
        fprintf(fp,
                "    {\n"
                "        .name = \"tw_g%d\",\n"
                "        .get = get_tw_g%d,\n"
                "        .set = set_tw_g%d,\n"
                "        .h = h_tw_g%d,\n"
                "        .quater_turns = %s,\n",
                i, i, i, i, quater_turns[i]);
        if (!sym)
            fprintf(fp,
                    "        .order = %lld,\n",
                    x[i].order);
        else
            fprintf(fp,
                    "        .is_sym = 1,\n"
                    "        .num_syms = 16,\n"
                    "        .coord_to_rep_sym = tw_g%d_coord_to_rep_sym,\n"
                    "        .coord_to_eqv_class = tw_g%d_coord_to_eqv_class,\n"
                    "        .eqv_class_to_rep = tw_g%d_eqv_class_to_rep,\n"
                    "        .get_sym_part = get_%s,\n"
                    "        .set_sym_part = set_%s,\n"
                    "        .sym_part_order = %lld,\n",
                    i, i, i, sym->ref->name, sym->ref->name, sym->ref->order);
        fprintf(fp,
                "    },\n");
    }
    fprintf(fp, "};\n");

    fclose(fp);

    fp = fopen("coord.h", "w");
    fprintf(fp,
            "#ifndef COORD_H\n"
            "#define COORD_H\n"
            "\n"
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
            "    // sym data\n"
            "    int is_sym;\n"
            "    long long sym_part_order;\n"
            "    int num_syms;\n"
            "    int *coord_to_rep_sym;\n"
            "    int *coord_to_eqv_class;\n"
            "    int *eqv_class_to_rep;\n"
            "    long long (*get_sym_part)(cube);\n"
            "    cube (*set_sym_part)(long long);\n"
            "} coord;\n");
    for (int i=0; i<num_sym_coords; ++i)
        fprintf(fp,
                "\n"
                "static int tw_g%d_coord_to_rep_sym[%lld];\n"
                "static int tw_g%d_coord_to_eqv_class[%lld];\n"
                "static int tw_g%d_eqv_class_to_rep[%lld];\n",
                0, sym_coords[i]->order,
                0, sym_coords[i]->order,
                0, sym_coords[i]->order);
    fprintf(fp,
            "\n"
            "static coord tw_coords[%d];\n", n);
    fprintf(fp,
            "\n"
            "#endif\n");
    fclose(fp);
}

int main(void)
{
    struct coord coord_eo = {
        .name = "flip",
        .type = RAW,
        .indexer = ORIENTATION,
        .subset = EDGES,
    };

    struct coord coord_co = {
        .name = "twist",
        .type = RAW,
        .indexer = ORIENTATION,
        .subset = CORNERS,
    };

    struct coord coord_ud_slice = {
        .name = "ud_slice",
        .type = RAW,
        .indexer = COMBINATION,
        .subset = SLICE_UD,
    };

    struct coord tw_coords[4] = {
#if 0
        [0] = coord_eo,
#else
        [0] = {
            .type = SYM,
            .ref = &coord_eo,
        },
#endif
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

    fp = fopen("coord.c", "w");
    write_coords(tw_coords, LENGTH(tw_coords), "tw");
    return 0;
}
