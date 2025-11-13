#ifndef SOLVER_H
#define SOLVER_H

static void solve(cube, int *, int *, int (*)(cube), int);
static void thistlethwaite(cube, int *, int *);
static void kociemba(cube, int *, int *);
static void optimal(cube, int *, int *);

#endif
