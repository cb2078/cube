#include "common.h"

#include "coord.h"
#include "cube.h"
#include "data.h"
#include "gui.h"
#include "moves.h"
#include "solver.h"
#include "table.h"
#include "util.h"

#include "coord.c"
#include "cube.c"
#include "data.c"
#include "gui.c"
#include "moves.c"
#include "solver.c"
#include "table.c"
#include "util.c"

#include <time.h>

enum state
{
    STATE_HELP,
    STATE_INVALID_ARG,
    STATE_PARSE_OPTIONS,
    STATE_RANDOM_SCRAMBLE,
    STATE_READ_SCRAMBLE,
    STATE_SOLVE,
};

int main(int argc, char **argv)
{
    cube_t x = new_cube();
    int moves[256], length;
    int i = 1;
    int random = 0;
    void (*solver)(cube_t, int *, int *) = optimal;
    srand(time(0));

    int state = i<argc ? STATE_PARSE_OPTIONS : STATE_HELP;
    for (;;)
    {
        switch (state)
        {
            case STATE_HELP:
                fprintf(stderr,
                        "Usage: cube [OPTION]... [MOVES]... \n"
                        "Example: cube --two-phase D L' F' U' L2 B L2 F' D R2 B2 D' R2 D F2\n"
                        "\n"
                        "Options:\n"
                        "--help        \tdisplay this help text and exit\n"
                        "--optimal     \tuse Cube Explorer's huge optimal solver (default)\n"
                        "--random      \tsolve a random scramble\n"
                        "--two-phase   \tuse Kociemba's two-phase algorithm\n"
                        "\n"
                        "Each solver uses pruning tables for improved performance; these can take between\n"
                        "10 minutes (for Kociemba's algorithm) and 1 hour (for the optimal solver) to\n"
                        "generate; once generated, they will be writeen to disk and reused on successive\n"
                        "runs.\n");
                return 0;

            case STATE_INVALID_ARG:
                fprintf(stderr, "cube: invliad arg -- '%s'\n", argv[i]);
                return 1;

            case STATE_PARSE_OPTIONS:
                if (i == argc || 0 != strncmp(argv[i], "--", 2))
                    state = random ? STATE_RANDOM_SCRAMBLE : STATE_READ_SCRAMBLE;
                else if (0 == strcmp(argv[i], "--help"))
                    state = STATE_HELP;
                else if (0 == strcmp(argv[i], "--two-phase"))
                    solver = kociemba, ++i;
                else if (0 == strcmp(argv[i], "--optimal"))
                    solver = optimal, ++i;
                else if (0 == strcmp(argv[i], "--random"))
                    random = 1, ++i;
                else
                    state = STATE_INVALID_ARG;
                break;

            case STATE_RANDOM_SCRAMBLE:
                if (i != argc)
                {
                    state = STATE_INVALID_ARG;
                }
                else
                {
                    int moves[256];
                    int length = LENGTH(moves);
                    make_scramble(moves, length);
                    x = apply_moves(x, moves, length);
                    state = STATE_SOLVE;
                }
                break;

            case STATE_READ_SCRAMBLE:
                if (i == argc)
                    state = STATE_SOLVE;
                else
                    x = apply_move(x, read_move(argv[i++]));
                break;

            case STATE_SOLVE:
                solver(x, moves, &length);
                print_moves(moves, length), putchar('\n');
                return 0;
        }
    }
}
