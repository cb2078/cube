#ifndef SOLVER_H
#define SOLVER_H

#include "coord.h"
#include "cube.h"

struct search_node
{
    cube_t cube;
    int move;
    int depth;
};

static void solve(cube_t, int *, int *, int (*)(cube_t));
static void optimal(struct coord *, cube_t, int *, int *);

#endif
