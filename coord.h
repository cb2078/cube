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
} coord;

#endif

static coord tw_coords[4];
