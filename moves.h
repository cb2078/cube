#ifndef MOVES_H
#define MOVES_H

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

static char *move_str[NUM_MOVES] =
{
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

static int move_set[18] =
{
    U,  R,  F,  D,  L,  B,
    U2, R2, F2, D2, L2, B2,
    U3, R3, F3, D3, L3, B3,
};

static int move_set[18];
static int prune_move(int, int);
static void possible_moves(int *, int *, int, int[6]);
static void print_moves(int *, int);
static void make_scramble(int *, int);
static void read_moves(char *s, int *, int *);
static int apply_cancellations(int *, int *);
static void inverse_moves(int *, int);

#endif
