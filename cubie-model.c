#include "common.h"
#include <cglm/quat.h>
#include <cglm/vec3-ext.h>

static int valid_move[NUM_MOVES] = {
    [U] =1, [R] =1, [F] =1, [D] =1, [L] =1, [B] =1,
    [U2]=1, [R2]=1, [F2]=1, [D2]=1, [L2]=1, [B2]=1,
    [U3]=1, [R3]=1, [F3]=1, [D3]=1, [L3]=1, [B3]=1,
};
static struct cubie_model move_table[NUM_MOVES];

void cm_from_vm(struct vector_model *vcube, struct cubie_model *ccube)
{
    int num_edges=0, num_corners=0;
    for (int i=0; i<NUM_CUBIES; ++i) {
        switch (vm_get_cubie_type(vcube, i)) {
            case CORNER:
                ccube->cp[num_corners] = vm_get_cubie_permutation(vcube, i);
                ccube->co[num_corners] = vm_get_cubie_orientation(vcube, i);
                ++num_corners;
                break;
            case EDGE:
                ccube->ep[num_edges] = vm_get_cubie_permutation(vcube, i);
                ccube->eo[num_edges] = vm_get_cubie_orientation(vcube, i);
                ++num_edges;
                break;
        }
    }
    assert(num_corners == 8);
    assert(num_edges == 12);
}

static void generate_move_table(void)
{
    struct vector_model cube, solved;
    vm_new(&solved);
    for (int move=0; move<NUM_MOVES; ++move)
    {
        if (!valid_move[move]) continue;
        cube = solved;
        vm_move(&cube, move);
        cm_from_vm(&cube, &move_table[move]);
    }
}

void cm_new(struct cubie_model *cube)
{
    static int initialised = 0;
    if (!initialised)
    {
        generate_move_table();
        initialised=1;
    }
    for (int i=0; i<NUM_CORNERS; ++i) cube->cp[i]=i, cube->co[i]=0;
    for (int i=0; i<NUM_EDGES;   ++i) cube->ep[i]=i, cube->eo[i]=0;
    return;
}

void cm_print(struct cubie_model *cube)
{
    printf("cubie model:\n");
    printf("    "); for (int i=0; i<NUM_EDGES;   ++i) printf("%2d ", i);           printf("\n");
    printf("CP: "); for (int i=0; i<NUM_CORNERS; ++i) printf("%2d ", cube->cp[i]); printf("\n");
    printf("CO: "); for (int i=0; i<NUM_CORNERS; ++i) printf("%2d ", cube->co[i]); printf("\n");
    printf("EP: "); for (int i=0; i<NUM_EDGES;   ++i) printf("%2d ", cube->ep[i]); printf("\n");
    printf("EO: "); for (int i=0; i<NUM_EDGES;   ++i) printf("%2d ", cube->eo[i]); printf("\n");
    printf("\n");
}

void cm_move(struct cubie_model *cube, int move)
{
    assert(valid_move[move]);
    for (int i=0; i<NUM_CORNERS; ++i)
    {
        cube->cp[i] = move_table[move].cp[cube->cp[i]];
        cube->co[i] += move_table[move].co[cube->cp[i]];
        cube->co[i] %= 3;
    }
    for (int i=0; i<NUM_EDGES; ++i)
    {
        cube->ep[i] = move_table[move].ep[cube->ep[i]];
        cube->eo[i] += move_table[move].eo[cube->ep[i]];
        cube->eo[i] %= 2;
    }
}
