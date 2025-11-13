#ifndef SOLVER_H
#define SOLVER_H

static void solve(cube_t, int *, int *, int (*)(cube_t), int);
static void thistlethwaite(cube_t, int *, int *);
static void kociemba(cube_t, int *, int *);
static void optimal(cube_t, int *, int *);

#endif
