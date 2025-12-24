#ifndef CUBE_TABLE_H
#define CUBE_TABLE_H

#include "cube.h"
#include "moves.h"

static const long long pow2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
static const long long pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};
static const long long fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600};

#include "data.inc"

static cube_t get_move_cube(int);
static cube_t get_sym_cube(int);

#endif
