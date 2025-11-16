#include "common.h"
#include "gen-coord.h"

static FILE *fp;

static long long coord_max(const struct coord *c)
{
    switch (c->type)
    {
        case RAW:
            ASSERT(c->max);
            return c->max;
        case TETRAD_TWIST:
            return 6;
        case COMP:
            long long max = 1;
            for (int i=0; i<(c->count?:2); ++i)
                max *= coord_max(&c->coords[i]);
            return max;
        case SYM_COMP:
            ASSERT(c->classes);
            return c->classes * coord_max(&c->coords[1]);
        default:
            UNREACHABLE();
    }
}

enum mode
{
    GET,
    SET,
};

static void write_coord_rec(const struct coord *c, int mode, int at_start, int at_end)
{
    int end = (c->pieces==CORNERS ? 8 : c->pieces == EDGES ? 12 : 4) - c->offset;
    char name[256], args[256], amount[256], result[256];

    static char *pieces_str[] =
    {
        "corners",
        "urf_tetrad",
        "urb_tetrad",
        "edges",
        "ud_slice",
        "rl_slice",
        "fb_slice",
    };

    char *get_pieces(void)
    {
        if (!c->offset)
            return pieces_str[c->pieces];
        static char buf[256];
        sprintf(buf, "%s+%d", pieces_str[c->pieces], c->offset);
        return buf;
    }

    void offset(void)
    {
        static int table[] = {0, 0, 4, 0, 0, 4, 8};
        if (c->offset || table[c->pieces])
            fprintf(fp, "    for (int i=%d; i<%d; ++i) x.%s[i] %c= %d;\n",
                    c->offset, end+c->offset, pieces_str[c->pieces], "-+"[mode==SET], c->offset+table[c->pieces]);
    }

    void zero_orientation(void)
    {
        if (c->ignore_orientation)
            fprintf(fp, "    set_%co(&x, 0);\n","ce"[c->pieces==EDGES]);
    }

    void shift_result(void)
    {
        if ((mode==GET && !at_start) || (mode==SET && !at_end))
            fprintf(fp, "    r %c= %s;\n", "*/"[mode==SET], amount);
    }

    void set_result(void)
    {
        if (mode==SET && !at_end)
            sprintf(result, ", r%%"), strcat(result, amount);
        else if (mode==SET)
            sprintf(result, ", r");
        else
            result[0] = '\0';
    }

    switch (c->type)
    {
        case RAW:
            switch (c->indexer)
            {
                case ORIENTATION:
                    sprintf(name, "%co", "ce"[c->pieces>=EDGES]);
                    sprintf(args, "%s", mode==GET ? "x" :"&x");
                    sprintf(amount, "pow%c[%d]", "32"[c->pieces>=EDGES], end-1);
                    break;
                case PERMUTATION:
                    sprintf(name, "permutation");
                    sprintf(args, "x.%s, %d", get_pieces(), end);
                    sprintf(amount, "fact[%d]", end);
                    break;
                case PARTIAL_PERMUTATION:
                    sprintf(name, "partial_permutation");
                    sprintf(args, "x.%s, %d, %d", get_pieces(), end, c->length);
                    sprintf(amount, "pick[%d][%d]", end, c->length);
                    break;
                case COMBINATION:
                    sprintf(name, "combination");
                    sprintf(args, "x.%s, %d, %d", get_pieces(), end, c->length);
                    sprintf(amount, "choose[%d][%d]", end, c->length);
                    break;
                default:
                    UNREACHABLE();
            }
            fprintf(fp, "\n");
            if (mode==GET) zero_orientation(), offset(), shift_result();
            set_result();
            fprintf(fp, "    %s_%s(%s%s);\n", mode==GET ? "r += get" : "set", name, args, result);
            if (mode==SET) shift_result(), offset();
            break;
        case TETRAD_TWIST:
            sprintf(amount, "6");
            fprintf(fp, "\n");
            if (mode==GET) shift_result();
            set_result();
            fprintf(fp, "    %s_tetrad_twist(%s%s);\n",
                    mode==GET ? "r += get" : "set",
                    mode==GET ? "x" : "&x",
                    result);
            if (mode==SET) shift_result();
            break;
        case COMP:
            for (int i=0; i<c->count; ++i)
                write_coord_rec(&c->coords[mode==GET ? i : c->count-1-i], mode, at_start && i==0, i==c->count-1);
            break;
        case SYM_COMP:
            fprintf(fp, "\n");
            if (mode==GET)
                fprintf(fp,
                        "    r += get_%s(x);\n"
                        "    x = apply_sym(x, %s_to_sym[r]);\n"
                        "    r = %s_to_class[r];\n",
                        c->coords[0].name, c->coords[0].name, c->coords[0].name);
            else
                fprintf(fp,
                        "    x = set_%s(%s_to_rep[r/%lld]);\n"
                        "    r %%= %lld;\n",
                        c->coords[0].name, c->coords[0].name, coord_max(&c->coords[1]), coord_max(&c->coords[1]));
            write_coord_rec(&c->coords[1], mode, 0, 1);
            break;
            // NOTE just check that get and set work for sym-comp since they
            // are done slightly differently to everything else
        default:
            UNREACHABLE();
    }
}

static void write_separator(void)
{
    fprintf(fp, "\n");
    for (int i=0; i<80; ++i)
        fprintf(fp, "/");
    fprintf(fp, "\n\n");
}

static void write_coord(char *name, int i, const struct coord *c, int classes)
{
    ASSERT(c->name);
    if (classes)
        fprintf(fp,
                "// TODO read/write these to disk\n"
                "static int %s_to_rep[%d];\n"
                "static int %s_to_class[%lld];\n"
                "static int %s_to_sym[%lld];\n"
                "static int %s_self_syms[%lld];\n"
                "\n",
                c->name, classes,
                c->name, coord_max(c),
                c->name, coord_max(c),
                c->name, coord_max(c));
    fprintf(fp,
            "static long long get_%s(cube_t x)\n"
            "{\n"
            "    long long r = 0;\n", c->name);
    write_coord_rec(c, GET, 1, 1);
    fprintf(fp,
            "\n"
            "    return r;\n"
            "}\n"
            "\n"
            "static cube_t set_%s(long long r)\n"
            "{\n", c->name);
    if (c->type==SYM_COMP)
        fprintf(fp, "    cube_t x;\n");
    else
        fprintf(fp, "    cube_t x = new_cube();\n");
    write_coord_rec(c, SET, 1, 1);
    fprintf(fp,
            "\n"
            "    return x;\n"
            "}\n");
    if (!classes)
        fprintf(fp,
                "\n"
                "static int h_%s(cube_t x)\n"
                "{\n"
                "    return table_get(%s_coords[%d].table, %s_coords[%d].get(x));\n"
                "}\n",
                c->name, name, i, name, i);
    fprintf(fp, "\n");
}

static void write_coords(char *name, const struct coord *coords, int n)
{
    void write_entry(const struct coord *c, char *indent)
    {
        fprintf(fp, "%s{\n",                   indent);
        fprintf(fp, "%s    .name = \"%s\",\n", indent, c->name);
        fprintf(fp, "%s    .get = get_%s,\n",  indent, c->name);
        fprintf(fp, "%s    .set = set_%s,\n",  indent, c->name);
        fprintf(fp, "%s    .max = %lld,\n",    indent, coord_max(c));
        if (c->type==SYM_COMP)
        {
            fprintf(fp, "%s    //\n",                         indent);
            fprintf(fp, "%s    .num_syms = %d,\n",            indent, c->num_syms);
            fprintf(fp, "%s    .classes = %d,\n",             indent, c->classes);
            fprintf(fp, "%s    .to_rep = %s_to_rep,\n",       indent, c->coords[0].name);
            fprintf(fp, "%s    .to_class = %s_to_class,\n",   indent, c->coords[0].name);
            fprintf(fp, "%s    .to_sym = %s_to_sym,\n",       indent, c->coords[0].name);
            fprintf(fp, "%s    .self_syms = %s_self_syms,\n", indent, c->coords[0].name);
            fprintf(fp, "%s    .base = &(struct coord)\n",    indent);
            write_entry(&c->coords[0], "        ");
        }
        if (strlen(indent)==4)
            fprintf(fp, "%s    .h = h_%s,\n",       indent, c->name);
        if (c->move_mask)
            fprintf(fp, "%s    .move_mask = %s,\n", indent, c->move_mask);
        fprintf(fp, "%s},\n", indent);
    }

    for (int i=0; i<n; ++i)
    {
        if (coords[i].type==SYM_COMP)
            write_coord(name, i, &coords[i].coords[0], coords[i].classes);
        write_coord(name, i, &coords[i], 0);
    }
    fprintf(fp,
            "static struct coord %s_coords[] =\n"
            "{\n", name);
    for (int i=0; i<n; ++i)
        write_entry(&coords[i], "    ");
    fprintf(fp, "};\n");
}

int main(void)
{
    fp = fopen("coord.c", "w");
    write_coords("thistlethwaite", thistlethwaite, LENGTH(thistlethwaite));
    write_separator();
    write_coords("kociemba", kociemba, LENGTH(kociemba));
    write_separator();
    write_coords("optimal", optimal, LENGTH(optimal));
    fclose(fp);

    fp = fopen("coord.h", "w");
    fprintf(fp,
            "#ifndef COORD_H\n"
            "#define COORD_H\n"
            "\n"
            "#include \"cube.h\"\n"
            "#include \"table.h\"\n"
            "\n"
            "struct coord\n"
            "{\n"
            "    char *name;\n"
            "    long long (*get)(cube_t);\n"
            "    cube_t (*set)(long long);\n"
            "    int (*h)(cube_t);\n"
            "    long long max;\n"
            "    int move_mask;\n"
            "    struct table *table;\n"
            "    //\n"
            "    int num_syms;\n"
            "    int classes;\n"
            "    int *to_rep;\n"
            "    int *to_class;\n"
            "    int *to_sym;\n"
            "    int *self_syms;\n"
            "    struct coord *base;\n"
            "};\n"
            "\n"
            "static struct coord thistlethwaite_coords[];\n"
            "static struct coord kociemba_coords[];\n"
            "static struct coord optimal_coords[];\n"
            "\n"
            "#endif\n");
    fclose(fp);

    return 0;
}
