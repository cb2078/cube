#include "cube.h"

typedef struct
{
    char *name;
    int (*get)(cube);
    cube (*set)(int);
    int (*h)(cube);
    int order;
    table table;
    int quater_turns[6];
} coord;

extern coord tw_coords[4];
