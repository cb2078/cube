#ifndef MOVES_H
#define MOVES_H

#define NUM_FACE_TURNS 18

enum move_mask
{
    EO_MASK  = 147492,
    DR_MASK  = 221238,
    HTR_MASK = 258111,
};

enum move_type
{
    FACE_TURN,
    WIDE_MOVE,
    ROTATION,
    SLICE_MOVE,
};

enum move
{
    U,  R,  F,  D,  L,  B,
    U2, R2, F2, D2, L2, B2,
    U3, R3, F3, D3, L3, B3,

    UW,  RW,  FW,  DW,  LW,  BW,
    UW2, RW2, FW2, DW2, LW2, BW2,
    UW3, RW3, FW3, DW3, LW3, BW3,

    Y,  X,  Z,  E,  M,  S,
    Y2, X2, Z2, E2, M2, S2,
    Y3, X3, Z3, E3, M3, S3,

    NUM_MOVES,
};

static char *move_str[NUM_MOVES] =
{
    "U",  "R",  "F",  "D",  "L",  "B",
    "U2", "R2", "F2", "D2", "L2", "B2",
    "U'", "R'", "F'", "D'", "L'", "B'",

    "UW",  "RW",  "FW",  "DW",  "LW",  "BW",
    "UW2", "RW2", "FW2", "DW2", "LW2", "BW2",
    "UW'", "RW'", "FW'", "DW'", "LW'", "BW'",

    "Y",  "X",  "Z",  "E",  "M",  "S",
    "Y2", "X2", "Z2", "E2", "M2", "S2",
    "Y'", "X'", "Z'", "E'", "M'", "S'",
};

static int move_type(int x);
static int move_axis(int x);
static int move_face(int x);
static int move_opposite_face(int x);
static int move_side(int x);
static int move_amount(int x);
static void possible_moves(int *moves, int *length, int move, int mask);
static void print_moves(int *moves, int length);
static void make_scramble(int *moves, int length);
static int read_move(char *s);
static void read_moves(char *s, int *moves, int *length);
static int apply_cancellations(int *moves, int *length);
static int inverse_move(int x);
static void inverse_moves(int *moves, int length);

#endif
