#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"
#include "util.h"

#define NUM_SYMS 48

#define CO_MAX pow3[7]
#define CSEP_MAX choose[8][4]
#define EO_MAX pow2[11]
#define ESEP_MAX (choose[12][4]*choose[8][4])
#define PARTIAL_EO_MAX pow2[EO_VARIANT]

#define CO_CSCP_MAX (CO_MAX*CSEP_MAX)
#define EO_ESEP_MAX (EO_MAX*ESEP_MAX)
#define PARTIAL_EO_ESEP_MAX (PARTIAL_EO_MAX*ESEP_MAX)

struct coord
{
    char filename[256];
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    int (*h)(cube_t);
    int (*h_optimal)(cube_t);
    long long max;
    // symmetric composite coordinates
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

static struct coord coord_eo_none;
static struct coord coord_eo_partial;
static struct coord coord_eo_full;

static void init_coord(struct coord *c);

static int EO_VARIANT = 1;

#endif
