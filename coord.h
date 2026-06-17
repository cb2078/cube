#ifndef COORD_H
#define COORD_H

#include "cube.h"
#include "data.h"
#include "table.h"

#define NUM_SYMS 48

#define CO_MAX pow3[7]
#define CSEP_MAX choose[8][4]
#define EO_MAX pow2[11]
#define ESEP_MAX (choose[12][4]*choose[8][4])
#define ORBIT_MAX (12*24*24*24*24)
#define PARTIAL_EO_MAX (1<<EO_VARIANT)

#define CO_CSEP_MAX (CO_MAX*CSEP_MAX)
#define EO_ESEP_MAX (EO_MAX*ESEP_MAX)
#define PARTIAL_EO_ESEP_MAX (PARTIAL_EO_MAX*ESEP_MAX)

#define CO_CSEP_CLASSES 3393ll
#define ORBIT_CLASSES 85556ll

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
    struct sym_coord *sym;
    unsigned *table;
    int bits;
};

static struct coord coord_phase1;
static struct coord coord_phase1_full;
static struct coord coord_phase2;
static struct coord coord_phase2_full;

static int is_self_sym(struct coord *c, cube_t x, int s);
static void init_sym(struct sym_coord *c);
static void init_coord(struct coord *c, void (*fill_prune_table)(void));

static int EO_VARIANT = -1;

static inline void set_eo_variant(int v)
{
    ASSERT(v >= 0 && v < NUM_EDGES);
    EO_VARIANT = v;
    coord_phase1.max = coord_phase1.sym->classes * PARTIAL_EO_ESEP_MAX;
}

#endif
