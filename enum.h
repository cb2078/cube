#pragma once

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

static char *move_str[] =
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

static char *cubie_str[] =
{
    "URF", "ULB", "DRB", "DLF",
    "URB", "ULF", "DRF", "DLB",
    "RF",  "RB",  "LF",  "LB",
    "UF",  "UB",  "DF",  "DB",
    "UR",  "UL",  "DR",  "DL",
};
