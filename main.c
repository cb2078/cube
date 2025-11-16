#include "common.h"

#include "coord.h"
#include "cube.h"
#include "data.h"
#include "gui.h"
#include "moves.h"
#include "prune.h"
#include "solver.h"
#include "table.h"
#include "util.h"

#include "coord.c"
#include "cube.c"
#include "data.c"
#include "gui.c"
#include "moves.c"
#include "prune.c"
#include "solver.c"
#include "table.c"
#include "util.c"

#include <time.h>

enum state
{
    STATE_HELP,
    STATE_INVALID_ARG,
    STATE_PARSE_OPTIONS,
    STATE_READ_SCRAMBLE,
    STATE_SOLVE,
};

int main(int argc, char **argv)
{
    cube_t x = new_cube();
    int i = 1;
    void (*solver)(cube_t, int *, int *) = optimal;
    srand(time(0));

    int state = i<argc ? STATE_PARSE_OPTIONS : STATE_HELP;
    for (;;)
    {
        switch (state)
        {
            case STATE_HELP:
                fprintf(stderr,
                        "Usage: cube [SOLVER] SCRAMBLE\n"
                        "Example: cube --two-phase D L' F' U' L2 B L2 F' D R2 B2 D' R2 D F2\n"
                        "\n"
                        "Miscellaneous:\n"
                        "--help        \tdisplay this help text and exit\n"
                        "\n"
                        "Solvers:\n"
                        "  --optimal   \tuse Cube Explorer's huge optimal solver (default)\n"
                        "  --two-phase \tuse Kociemba's two-phase algorithm\n"
                        "  --four-phase\tuse Thistlethwaite's algorithm\n"
                        "\n"
                        "Each solver uses pruning tables for improved performance; these can take between\n"
                        "1 second (for Thistlethwaite's algorithm) and 1 hour (for the optimal solver) to\n"
                        "generate; once generated, they will be writeen to disk and reused on successive\n"
                        "runs.\n");
                return 0;

            case STATE_INVALID_ARG:
                fprintf(stderr, "cube: invliad arg -- '%s'\n", argv[i]);
                return 1;

            case STATE_PARSE_OPTIONS:
                if (i == argc)
                    return 0;
                else if (0 != strncmp(argv[i], "--", 2))
                    state = STATE_READ_SCRAMBLE;
                else if (0 == strcmp(argv[i], "--help"))
                    state = STATE_HELP;
                else if (0 == strcmp(argv[i], "--four-phase"))
                    solver = thistlethwaite, ++i;
                else if (0 == strcmp(argv[i], "--two-phase"))
                    solver = kociemba, ++i;
                else if (0 == strcmp(argv[i], "--optimal"))
                    solver = optimal, ++i;
                else
                    state = STATE_INVALID_ARG;
                break;

            case STATE_READ_SCRAMBLE:
                if (i == argc)
                    state = STATE_SOLVE;
                else
                    x = apply_move(x, read_move(argv[i++]));
                break;

            case STATE_SOLVE:
                int moves[256], length;
                solver(x, moves, &length);
                print_moves(moves, length), putchar('\n');
                return 0;
        }
    }
}
