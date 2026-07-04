#pragma once

#include "cube.h"
#include "data.h"
#include "table.h"

#define NUM_SYMS 48
#define INFO_BITS 32
#define SYM_BITS 6
static_assert(1<<SYM_BITS >= NUM_SYMS && 1<<SYM_BITS-1 < NUM_SYMS);

#define CO_MAX pow3[7]
#define CSEP_MAX choose[8][4]
#define EO_MAX pow2[11]
#define ESEP_MAX (choose[12][4]*choose[8][4])
#define CP_MAX fact[8]
#define EP_MAX fact[12]

#define CO_CSEP_MAX (CO_MAX*CSEP_MAX)
#define EO_ESEP_MAX (EO_MAX*ESEP_MAX)

#define CO_CSEP_CLASSES 3393ll
#define ORBIT_CLASSES 85556ll

#define SYM_COORD_NAME "co_csep"
#define SYM_COORD_CLASSES CO_CSEP_CLASSES
#define SYM_COORD_MAX CO_CSEP_MAX
static_assert(SYM_COORD_CLASSES < 1<<INFO_BITS-SYM_BITS);

#define COORD_NAME ""
#define COORD_MAX (SYM_COORD_CLASSES*(ESEP_MAX<<EO_VARIANT))

static struct 
{
    struct
    {
        unsigned class: INFO_BITS-SYM_BITS;
        unsigned sym: SYM_BITS;
    } *info;
    int *to_rep;
    long long *self_syms;
} sym_coord;

static struct
{
    unsigned *table;
    int bits;
} coord;

static inline long long get_sym_coord(cube_t);
static inline cube_t set_sym_coord(long long);
static inline long long get_coord(cube_t, int);
static inline cube_t set_coord(long long, int);

static void init_sym(void);
static void init_coord(void);

static int EO_VARIANT;

static inline int is_self_sym(cube_t x, int s)
{
    return sym_coord.self_syms[get_sym_coord(x)] >> s & 1;
}