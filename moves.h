#pragma once

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
