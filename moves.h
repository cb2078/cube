#ifndef MOVES_H
#define MOVES_H

enum move_mask
{
    EO_MASK  = 147492,
    DR_MASK  = 221238,
    HTR_MASK = 258111,
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

static int move_face(int x);
static int move_axis(int x);
static int move_side(int x);
static int move_amount(int x);
static void transform_moves(int *moves, int length, int sym);
static void possible_moves(int *moves, int *length, int move, int mask);
static void print_moves(int *moves, int length);
static void make_scramble(int *moves, int length);
static void read_moves(char *s, int *moves, int *length);
static int apply_cancellations(int *moves, int *length);
static void inverse_moves(int *moves, int length);

#endif
