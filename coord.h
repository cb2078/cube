#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"

struct coord
{
    char *name;
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    int (*h)(cube_t);
    long long max;
    int move_mask;
    struct table *table;
    //
    int num_syms;
    int classes;
    int *to_rep;
    int *to_class;
    int *to_sym;
    int *self_syms;
    struct coord *base;
};

static struct coord thistlethwaite_coords[];
static struct coord kociemba_coords[];
static struct coord optimal_coords[];

#endif
