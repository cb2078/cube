#include "cube.c"
#include "time.h"

int main(void)
{
    srand(time(0));
    int moves[55];
    int length=10;
    make_scramble(moves, length);
    printf("scrambe: ");
    print_moves(moves, length);
    printf(" // %d move%s\n", length, length==1?"":"s");
    cube x = apply_moves(new_cube(), moves, length);
    search(x, moves, &length);
    int cancellations = apply_cancellations(moves, &length);
    printf("solution: ");
    print_moves(moves, length);
    printf(" // %d move%s\n", length, length==1?"":"s");

    return 0;
}
