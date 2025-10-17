#pragma once

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(x) ((x)<0 ? -(x) : x)
#define LENGTH(x) (long signed int)(sizeof(x)/sizeof(x[0]))
#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)

#define NUM_CUBIES 27
#define NUM_CORNERS 8
#define NUM_EDGES 12

enum cubie_type {CORNER, EDGE, CENTRE};

enum move_type
{
    FACE_TURN,
    WIDE_MOVE,
    ROTATION,
    SLICE_MOVE,
};

enum move
{
    U, R, F, D, L, B,
    UW, RW, FW, DW, LW, BW,
    Y, X, Z, E, M, S,

    U2, R2, F2, D2, L2, B2,
    UW2, RW2, FW2, DW2, LW2, BW2,
    Y2, X2, Z2, E2, M2, S2,

    U3, R3, F3, D3, L3, B3,
    UW3, RW3, FW3, DW3, LW3, BW3,
    Y3, X3, Z3, E3, M3, S3,

    NUM_MOVES,
};

enum cubies
{
    URF, ULB, DRB, DLF,
    URB, ULF, DRF, DLB,
    RF,  RB,  LF,  LB,
    UF,  UB,  DF,  DB,
    UR,  UL,  DR,  DL,
};

static char *move_str[] = {
    "U", "R", "F", "D", "L", "B",
    "UW", "RW", "FW", "DW", "LW", "BW",
    "Y", "X", "Z", "E", "M", "S",

    "U2", "R2", "F2", "D2", "L2", "B2",
    "UW2", "RW2", "FW2", "DW2", "LW2", "BW2",
    "Y2", "X2", "Z2", "E2", "M2", "S2",

    "U'", "R'", "F'", "D'", "L'", "B'",
    "UW'", "RW'", "FW'", "DW'", "LW'", "BW'",
    "Y'", "X'", "Z'", "E'", "M'", "S'",
};

static char *cubie_str[] = {
    "URF", "ULB", "DRB", "DLF",
    "URB", "ULF", "DRF", "DLB",
    "RF",  "RB",  "LF",  "LB",
    "UF",  "UB",  "DF",  "DB",
    "UR",  "UL",  "DR",  "DL",
};

static inline int get_move_type(int move)
{
    move%=U2;
    if (move>=E)
        return SLICE_MOVE;
    if (move>=Y)
        return ROTATION;
    if (move>=UW)
        return WIDE_MOVE;
    return FACE_TURN;
}

// cube.c
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
cube apply_move(cube, int);
cube apply_moves(cube, int *, int);
//
int get_co(cube);
int get_eo(cube);
void set_co(cube *, int);
void set_eo(cube *, int);
int get_tetrad_twist(cube);
void set_tetrad_twist(cube *, int);

// moves.c
extern int move_set[18];
int prune_move(int, int);
void possible_moves(int *, int *, int, int[6]);
void print_moves(int *, int);
void make_scramble(int *, int);
void read_moves(char *s, int *, int *);
int apply_cancellations(int *, int *);
void inverse_moves(int *, int);

// gui.c
void gui(void);
void gui_show_moves(int *, int);
void gui_show_moves_fast(int *, int);
void gui_show_cube(cube);
void gui_wait_for_close();

// table.c
typedef struct
{
    int size; // size in bytes
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    char filename[256];
    unsigned *data;
} table;
table table_new(int size, int bits, char *filename);
int table_read(table t);
int table_write(table t);
void table_set(table t, int i, int x);
int table_get(table t, int i);

//
table init_tetrad_twist_table(void);
extern table tetrad_twist_table;
void thistlethwaite(cube x, int *path, int *length);
