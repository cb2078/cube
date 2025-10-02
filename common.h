#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(x) ((x)<0 ? -(x) : x)
#define LENGTH(x) (sizeof(x)/sizeof(x[0]))
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
    UR,  UL,  DR,  DL,
    UF,  UB,  DF,  DB,
    RF,  RB,  LF,  LB,
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
    "UR",  "UL",  "DR",  "DL",
    "UF",  "UB",  "DF",  "DB",
    "RF",  "RB",  "LF",  "LB",
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
        char ur, ul, dr, dl;
        char uf, ub, df, db;
        char rf, rb, lf, lb;
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
} cube;
cube new_cube(void);
cube apply_move(cube, int);
cube apply_moves(cube, int *, int);
void solve(cube, int *, int *);

// moves.c
extern int move_set[18];
int prune_move(int, int);
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
