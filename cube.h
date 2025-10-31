#pragma once

#define NUM_CORNERS 8
#define NUM_EDGES 12

typedef union
{
    struct
    {
        char urf, ulb, drb, dlf;
        char urb, ulf, drl, dlb;
        char rf, rb, lf, lb;
        char uf, ub, df, db;
        char ur, ul, dr, dl;
    };
    struct
    {
        char corners[NUM_CORNERS];
        char edges[NUM_EDGES];
    };
    struct
    {
        char cubies[NUM_CORNERS+NUM_EDGES];
    };
    struct
    {
        char tetrads[2][4];
        char slices[3][4];
    };
    struct
    {
        char orbits[5][4];
    };
} cube;

enum tetrad
{
    TETRAD_URF,
    TETRAD_URB,
};

enum slice
{
    SLICE_UD,
    SLICE_RL,
    SLICE_FB,
};

cube new_cube(void);
void print_cube(cube);
int cube_eq(cube, cube);
cube compose(cube, cube);
cube compose_3(cube, cube, cube);
cube apply_move(cube, int);
cube apply_moves(cube, int *, int);
cube invert_co(cube);

int get_co(cube);
int get_eo(cube);
void set_co(cube *, long long);
void set_eo(cube *, long long);
int get_tetrad_twist(cube);
void set_tetrad_twist(cube *, int);
void thistlethwaite(cube x, int *path, int *length);
