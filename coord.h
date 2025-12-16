#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"
#include "util.h"

#define CO_MAX pow3[7]
#define CSEP_MAX choose[8][4]
#define EO_MAX pow2[11]
#define ESEP_MAX (choose[12][4]*choose[8][4])
#define PARTIAL_EO_MAX pow2[EO_VARIANT]

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

static struct coord coord_partial_eo;

#endif
