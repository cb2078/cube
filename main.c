#include "common.h"
#include <time.h>

int main(void)
{
    srand(time(0));
    tetrad_twist_table = init_tetrad_twist_table();

    int moves[64];
    int length=100;

    make_scramble(moves, length);
    printf("scramble: "), print_moves(moves, length), putchar('\n');
    gui_show_moves_fast(moves, length);
    cube x = apply_moves(new_cube(), moves, length);
    thistlethwaite(x, moves, &length);
    gui_show_moves_fast(moves, length);
    gui_wait_for_close();

    return 0;
}
