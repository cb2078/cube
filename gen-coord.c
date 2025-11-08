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
    char *move_mask;
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
                TETRAD_URF,
                TETRAD_URB,
                EDGES,
                SLICE_UD,
                SLICE_RL,
                SLICE_FB,
            } subset;
            int k;
            int length;
        };
        struct
        {
            struct coord *ref;
            int symmetries[4];
            long long eqv_classes;
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

static void write_coord(struct coord *x, enum mode mode);
static void write_coord_rec(struct coord *x, enum mode mode, int at_start, int at_end)
{
#define S subsets[x->subset]
    int length;

    void write_offset(char op)
    {
        if (!S.start)
            return;
        int n = x->indexer==PERMUTATION ? length : S.end-S.start;
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
            length=x->length?:S.length, write_get_offset();
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
                    sprintf(expr, "permutation(x.%s, %d", S.name, length);
                    sprintf(amount, "fact[%d]", length);
                    x->order = fact[length];
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
            assert(at_start);
            x->order = x->eqv_classes ?: x->ref->order;
            if (mode == GET)
            {
                fprintf(fp, "    result = get_%s(x);\n", x->ref->name);
                if (!at_end)
                    fprintf(fp, "    x = apply_sym(x, %s_coord_to_rep_sym[result]);\n", x->ref->name);
                fprintf(fp, "    result = %s_coord_to_eqv_class[result];\n", x->ref->name);
            }
            else
            {
                fprintf(fp, "    i = %s_eqv_class_to_rep[result", x->ref->name);
                if (!at_end)
                    fprintf(fp, "%%%lld", x->eqv_classes);
                fprintf(fp, "];\n    x = set_%s(i);\n", x->ref->name);
            }
            if (!at_end)
                fprintf(fp,
                        "    %s= %lld;\n"
                        "\n", mode==GET ? "i *" : "result /",x->eqv_classes);
            // since this function is ran twice for get and set, only do the following once
            if (mode == GET)
            {
                assert(x->ref->order);
                sym_coords[num_sym_coords++] = x->ref;
            }
            break;
        case COMPOSITE:
            assert(x->count>1);
            // data[i][j] i=0 for corner, i=1 for edge, j=0 for permutation, j=1 for orientation
            int data[2][2] = {0};
            int conflict = 0;
            for (int i=0; i<x->count; ++i)
            {
                data[x->coords[i].subset >= EDGES][x->coords[i].indexer == ORIENTATION]++;
                if (conflict |= (data[0][0] && data[0][1]==1) || (data[1][0] && data[1][1]==1))
                {
                    if (mode == GET)
                        fprintf(fp,
                                "    for (int i=0; i<20; ++i) x.cubies[i] &= 0x0f;\n"
                                "\n");
                    else
                        fprintf(fp,
                                "    cube y = x;\n"
                                "    x = new_cube();\n"
                                "\n");
                }
                write_coord_rec(&x->coords[i], mode, i==0, i==x->count-1);
                x->order *= x->coords[i].order;
            }
            if (mode == SET && conflict)
                fprintf(fp, "    x = compose(x, y);\n");
            break;
    }
#undef S
}

static void write_coord(struct coord *x, enum mode mode)
{
    write_coord_rec(x, mode, 1, 1);
}

static void write_coord_getter_and_setter(struct coord *x, char *name)
{
    fprintf(fp,
            "static long long get_%s(cube x)\n"
            "{\n"
            "    long long result=0%s;\n"
            "\n",
            name,
            x->type==COMPOSITE || (x->type==SYM && x->ref->type==COMPOSITE) ? ", i=1" : "");
    write_coord(x, GET);
    fprintf(fp,
            "    return result;\n"
            "}\n"
            "\n"
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
    for (int i=0; i<n; ++i)
    {
        char buf[256];
        snprintf(buf, 256, "%s_g%d", name, i);
        write_coord_getter_and_setter(x+i, buf);
        fprintf(fp,
                "static int h_%s_g%d(cube x)\n"
                "{\n"
                "    return table_get(tw_coords[%d].table, tw_coords[%d].get(x));\n"
                "}\n"
                "\n", name, i, i, i);
    }

    fprintf(fp,
            "static coord %s_coords[] =\n"
            "{\n", name);
    for (int i=0, j=0; i<n; ++i)
    {
        struct coord *sym = x[i].type==SYM ? &x[i]
            : x[i].type==COMPOSITE && x[i].coords[0].type==SYM ? &x[i].coords[0]
            : 0;
        fprintf(fp,
                "    {\n"
                "        .name = \"%s_g%d\",\n"
                "        .get = get_%s_g%d,\n"
                "        .set = set_%s_g%d,\n"
                "        .h = h_%s_g%d,\n"
                "        .order = %lld,\n",
                name, i,
                name, i,
                name, i,
                name, i,
                x[i].order);
        if (x[i].move_mask)
            fprintf(fp, "        .move_mask = %s,\n", x[i].move_mask);
        if (sym)
            fprintf(fp,
                    "        .is_sym = 1,\n"
                    "        .num_syms = 16,\n"
                    "        .eqv_classes = %lld,\n"
                    "        .self_syms = %s_self_syms,\n"
                    "        .coord_to_rep = %s_coord_to_rep,\n"
                    "        .coord_to_rep_sym = %s_coord_to_rep_sym,\n"
                    "        .coord_to_eqv_class = %s_coord_to_eqv_class,\n"
                    "        .eqv_class_to_rep = %s_eqv_class_to_rep,\n"
                    "        .get_sym_part = get_%s,\n"
                    "        .set_sym_part = set_%s,\n"
                    "        .sym_part_order = %lld,\n",
                    sym->eqv_classes,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->name,
                    sym->ref->order);
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
            "    int move_mask;\n"
            "    // sym data\n"
            "    int is_sym;\n"
            "    int num_syms;\n"
            "    long long eqv_classes;\n"
            "    long long sym_part_order;\n"
            "    long long *self_syms;\n"
            "    long long *coord_to_rep;\n"
            "    long long *coord_to_rep_sym;\n"
            "    long long *coord_to_eqv_class;\n"
            "    long long *eqv_class_to_rep;\n"
            "    long long (*get_sym_part)(cube);\n"
            "    cube (*set_sym_part)(long long);\n"
            "} coord;\n");
    for (int i=0; i<num_sym_coords; ++i)
        fprintf(fp,
                "\n"
                "static long long %s_self_syms[%lld];\n"
                "static long long %s_coord_to_rep[%lld];\n"
                "static long long %s_coord_to_rep_sym[%lld];\n"
                "static long long %s_coord_to_eqv_class[%lld];\n"
                "static long long %s_eqv_class_to_rep[%lld];\n",
                sym_coords[i]->name, sym_coords[i]->order,
                sym_coords[i]->name, sym_coords[i]->order,
                sym_coords[i]->name, sym_coords[i]->order,
                sym_coords[i]->name, sym_coords[i]->order,
                sym_coords[i]->name, sym_coords[i]->order);
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

    struct coord coord_flip_ud_slice = {
        .name = "flip_ud_slice",
        .type = COMPOSITE,
        .count = 2,
        .coords = (struct coord[]){
            [0] = coord_eo,
            [1] = coord_ud_slice,
        },
    };

    struct coord coord_cp = {
        .name = "cp",
        .type = RAW,
        .indexer = PERMUTATION,
        .subset = CORNERS,
    };

    struct coord *sym_coords[] = {
        &coord_flip_ud_slice,
        &coord_cp,
    };

    struct coord tw_coords[] = {
#if 0
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
            .move_mask = "DR_MASK",
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
            .move_mask = "HTR_MASK",
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
#else
        [0] = {
            .type = COMPOSITE,
            .count = 2,
            .coords = (struct coord []) {
                [0] = {
                    .type = SYM,
                    .eqv_classes = 64430,
                    .ref = &coord_flip_ud_slice,
                },
                [1] = coord_co,
            },
        },
        [1] = {
            .type = COMPOSITE,
            .move_mask = "DR_MASK",
            .count = 2,
            .coords = (struct coord []) {
                [0] = {
                    .type = SYM,
                    .eqv_classes = 2768,
                    .ref = &coord_cp,
                },
                [1] = {
                    .type = RAW,
                    .indexer = PERMUTATION,
                    .subset = SLICE_RL,
                    .length = 8,
                },
            },
        }
#endif
    };

    fp = fopen("coord.c", "w");
    for (int i=0; i<LENGTH(sym_coords); ++i)
        write_coord_getter_and_setter(sym_coords[i], sym_coords[i]->name);
    write_coords(tw_coords, LENGTH(tw_coords), "tw");
    return 0;
}
