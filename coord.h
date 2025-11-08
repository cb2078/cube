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
    int move_mask;
    // sym data
    int is_sym;
    int num_syms;
    long long eqv_classes;
    long long sym_part_order;
    long long *self_syms;
    long long *coord_to_rep;
    long long *coord_to_rep_sym;
    long long *coord_to_eqv_class;
    long long *eqv_class_to_rep;
    long long (*get_sym_part)(cube);
    cube (*set_sym_part)(long long);
} coord;

static long long flip_ud_slice_self_syms[1013760];
static long long flip_ud_slice_coord_to_rep[1013760];
static long long flip_ud_slice_coord_to_rep_sym[1013760];
static long long flip_ud_slice_coord_to_eqv_class[1013760];
static long long flip_ud_slice_eqv_class_to_rep[1013760];

static long long cp_self_syms[40320];
static long long cp_coord_to_rep[40320];
static long long cp_coord_to_rep_sym[40320];
static long long cp_coord_to_eqv_class[40320];
static long long cp_eqv_class_to_rep[40320];

static coord tw_coords[2];

#endif
