#include "cube.h"

typedef struct
{
    char *name;
    long long (*get)(cube);
    cube (*set)(long long);
    int (*h)(cube);
    long long order;
    table table;
    int quater_turns[6];
} coord;

extern coord tw_coords[4];
