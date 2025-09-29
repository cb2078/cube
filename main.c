#include "cube.c"

int main(void)
{
    int moves[] = {
        // U,R2,F,B,R,B2,R,U2,L,B2,R,U3,D3,R2,F,R3,L,B2,U2,F2
        U, R, B2, U2, F, L, R
    };
    cube x = apply_moves(new_cube(), moves, LENGTH(moves));
    int path[55];
    int length = search(x, path);
    for (int i=0; i<length; ++i) printf("%s ", move_str[path[i]]);
    printf("\n");

    return 0;
}
