#include "common.h"
#include <time.h>

int main(void)
{
#if 0
    srand(time(0));

    int moves[128];
    int length=100;

    make_scramble(moves, length);
    printf("scramble: "), print_moves(moves, length), putchar('\n');
    gui_show_moves_fast(moves, length);
    cube x = apply_moves(new_cube(), moves, length);
    thistlethwaite(x, moves, &length);
    gui_show_moves_fast(moves, length);
    gui_wait_for_close();
#else
    gui();
    gui_wait_for_close();
#endif

    return 0;
}
