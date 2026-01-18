#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "table.h"

#define NUM_SYMS 96

#define EP_MAX fact[12]
#define CP_MAX fact[8]
#define CO_MAX pow3[7]
#define CSEP_MAX choose[8][4]
#define EO_MAX pow2[11]
#define ESEP_MAX (512*choose[8][4])
#define PARTIAL_EO_MAX pow2[EO_VARIANT]

#define EO_EP_MAX (EO_MAX*EP_MAX)
#define CO_CSEP_MAX (CO_MAX*CSEP_MAX)
#define EO_ESEP_MAX (EO_MAX*ESEP_MAX)
#define PARTIAL_EO_ESEP_MAX (PARTIAL_EO_MAX*ESEP_MAX)

struct raw_coord
{
    char *name;
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    long long max;
};

struct sym_coord
{
    char *name;
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    long long max;
    long long classes;
    struct
    {
        unsigned class: 16;
        unsigned sym: 16;
    } *info;
    int *to_rep;
    long long *self_syms;
};

struct coord
{
    char *name;
    long long (*get)(cube_t);
    cube_t (*set)(long long);
    long long max;
    int (*h)(cube_t);
    int (*h_optimal)(cube_t);
    struct raw_coord *raw;
    struct sym_coord *sym;
    unsigned *table;
};

static struct sym_coord sym_co_csep;

static struct coord coord_phase1;
static struct coord coord_phase1_full;

static int is_self_sym(struct coord *c, cube_t x, int s);
static void init_coord(struct coord *c);

static int EO_VARIANT = 1;

#endif
