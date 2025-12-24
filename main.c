#include "common.h"

#include "coord.h"
#include "cube.h"
#include "data.h"
#include "map.h"
#include "moves.h"
#include "prune.h"
#include "solver.h"
#include "table.h"
#include "util.h"

#include "coord.c"
#include "cube.c"
#include "data.c"
#include "map.c"
#include "moves.c"
#include "prune.c"
#include "solver.c"
#include "table.c"
#include "util.c"

#include <time.h>

struct arg
{
    char *long_name;
    char short_name;
    enum
    {
        VALUE_NONE,
        VALUE_REQUIRED,
    } value_type;
    char *doc;
};

static struct arg args[] =
{
    {"eo",       'e', VALUE_REQUIRED, "edge orientation variant"},
    {"help",     'h', VALUE_NONE,     "print this help message and exit"},
    {"no-input", 'n', VALUE_NONE,     "ignore input, just generate the prune table"},
    {"random",   'r', VALUE_REQUIRED, "solve a NUM random move scramble"},
    {"threads",  't', VALUE_REQUIRED, "use NUM threads"},
    {"verbose",  'v', VALUE_NONE,     ""},
};

int main(int argc, char **argv)
{
    srand(time(0));

    struct coord *coord = &coord_eo_partial;
    int moves[256], length=0;
    int threads;
    cube_t x = new_cube();
    int i=1, j=0, val=0;

    void help(void)
    {
        static char *value_str[] = {"", "=NUM"};

        int width(int i)
        {
            return strlen(args[i].long_name)+strlen(value_str[args[i].value_type]);
        }

        fprintf(stderr,
                "Usage: cube [OPTION]...\n"
                "Read scrambles from standard input and write solution to standard output.\n"
                "\n"
                "Options:\n");
        int n = 0;
        for (int i=0; i<LENGTH(args); ++i)
            n = MAX(n, width(i));
        for (int i=0; i<LENGTH(args); ++i)
            fprintf(stderr, "  -%c, --%s%s%.*s\t%s\n",
                    args[i].short_name,
                    args[i].long_name, value_str[args[i].value_type],
                    n-width(i), "                    ",
                    args[i].doc);
        fprintf(stderr,
                "\n"
                "By default, all threads are used and -e1 is set.\n"
                "\n"
                "Examples:\n"
                "  echo \"R U R' U'\" | %s -e5\n"
                "  %s -t2 -e5 < scrambles.txt\n"
                "  %s -n -e3\n",
                argv[0], argv[0], argv[0]);
        }

    __attribute__((noreturn))
    void invalid_arg(void)
    {
        ERROR("invalid arg '%s'\n", argv[i]);
    }

    __attribute__((noreturn))
    void missing_arg_value(void)
    {
        ERROR("missing arg value '%s'\n", argv[i]);
    }

    void set_val(void)
    {
        char *end;
        if (!strlen(argv[i]+j))
            missing_arg_value();
        if (((!(val=strtoul(argv[i]+j, &end, 10)) &&
              strcmp(argv[i]+j, "0")) ||
             end-argv[i] != (long long)strlen(argv[i])))
            ERROR("invalid arg value '%s'\n", argv[i]+j);
        j = strlen(argv[i]);
    }

    char get_short_arg(void)
    {
        for (int k=0; k<LENGTH(args); ++k)
            if (args[k].short_name == argv[i][j])
            {
                ++j;
                if (args[k].value_type == VALUE_REQUIRED)
                    set_val();
                return args[k].short_name;
            }
        invalid_arg();
    }

    char get_long_arg(void)
    {
        for (int k=0; k<LENGTH(args); ++k)
            if (!strncmp(args[k].long_name, argv[i]+j, strlen(args[k].long_name)))
            {
                j += strlen(args[k].long_name);
                if (args[k].value_type == VALUE_REQUIRED)
                    argv[i][j] == '=' ? ++j, set_val() : missing_arg_value();
                else if (argv[k][j] == '=')
                    ERROR("unexpected argument value '%s'\n", argv[i]+j+1);
                else if (argv[i][j] != '\0')
                    invalid_arg();
                else
                    j = strlen(argv[i]);
                return args[k].short_name;
            }
        invalid_arg();
    }

    char get_arg()
    {
        if (i == argc)
            return 0;
        if (j == (long long)strlen(argv[i]))
            return ++i, j=0, get_arg();
        if (j)
            return get_short_arg();
        if (!j && !strncmp(argv[i], "--", 2))
            return j+=2, get_long_arg();
        if (argv[i][0] == '-')
            return ++j, get_short_arg();
        invalid_arg();
    }

    for (char c; (c=get_arg());)
        switch (c)
        {
            case 'e':
                if (val == 0)
                    coord = &coord_eo_none;
                else if (val < 11)
                    coord = &coord_eo_partial;
                else if (val == 11)
                    coord = &coord_eo_full;
                else
                    ERROR("invalid EO variant '%d'\n", val);
                EO_VARIANT = val;
                break;
            case 'h':
                help();
                return 0;
            case 'n':
                NO_INPUT = 1;
                break;
            case 'r':
                length = val;
                make_scramble(moves, length);
                x = apply_moves(x, moves, length);
                break;
            case 't':
                THREADS = val;
                break;
            case 'v':
                VERBOSE = 1;
                break;
            default:
                UNREACHABLE();
        }

#if DEBUG
    LOG("build mode: DEBUG\n");
#else
    LOG("build mode: RELEASE\n");
#endif
    if (NO_INPUT)
    {
        init_coord(coord);
    }
    else if (!length)
    {
        char buf[256];
        while (fgets(buf, LENGTH(buf), stdin))
        {
            read_moves(buf, moves, &length);
            x = apply_moves(new_cube(), moves, length);
            optimal(coord, x, moves, &length);
            print_moves(moves, length), putchar('\n');
        }
    }
    else
    {
        optimal(coord, x, moves, &length);
        print_moves(moves, length), putchar('\n');
    }

    return 0;
}
