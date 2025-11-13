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

int main(void)
{
    srand(time(0));

    int moves[128];
    int length=15;

    make_scramble(moves, length);
    printf("scramble: "), print_moves(moves, length), putchar('\n');
    cube x = apply_moves(new_cube(), moves, length);
    // thistlethwaite(x, moves, &length);
    kociemba(x, moves, &length);
    // optimal(x, moves, &length);

    return 0;
}
