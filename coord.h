#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"

struct coord
{
    char *filename;
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    int (*h)(cube_t);
    long long max;
    unsigned move_mask;
    // symmetric composite coordinates
    int num_syms;
    struct
    {
        long long (*get)(cube_t);
        cube_t (*set)(long long);
        long long max;
        long long classes;
    } sym;
    struct
    {
        long long (*get)(cube_t);
        cube_t (*set)(long long);
        long long max;
    } raw;
    // data
    struct table *table;
    int *to_rep;
    int *to_class;
    int *to_sym;
    long long *self_syms;
};

static struct coord coord_optimal;
static void init_coord(struct coord *c);

#endif
