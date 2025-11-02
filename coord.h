#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"

typedef struct
{
    char *name;
    long long (*get)(cube);
    cube (*set)(long long);
    int (*h)(cube);
    long long order;
    table *table;
    int quater_turns[6];
    // sym data
    int is_sym;
    long long sym_part_order;
    int num_syms;
    int *coord_to_rep_sym;
    int *coord_to_eqv_class;
    int *eqv_class_to_rep;
    long long (*get_sym_part)(cube);
    cube (*set_sym_part)(long long);
} coord;

static int tw_g0_coord_to_rep_sym[2048];
static int tw_g0_coord_to_eqv_class[2048];
static int tw_g0_eqv_class_to_rep[2048];

static coord tw_coords[4];

#endif
