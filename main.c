#include "common.h"

#include "coord.h"
#include "cube-table.h"
#include "cube.h"
#include "moves.h"
#include "util.h"
#include "table.h"
#include "gui.h"

#include "coord.c"
#include "cube-table.c"
#include "cube.c"
#include "moves.c"
#include "util.c"
#include "table.c"
#include "gui.c"

#include <time.h>

int main(void)
{
    srand(time(0));

    int moves[128];
    int length=15;

    make_scramble(moves, length);
    printf("scramble: "), print_moves(moves, length), putchar('\n');
    gui_show_moves_fast(moves, length);
    cube x = apply_moves(new_cube(), moves, length);
    optimal(x, moves, &length);
    gui_show_moves(moves, length);
    gui_wait_for_close();
    return 0;
}
