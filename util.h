#ifndef UTIL_H
#define UTIL_H

#include "util.inc"

static const long long pow2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
static const long long pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};
static const long long fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600};

static int get_combination(char *x, int n, int k);
static void set_combination(char *x, int n, int k, int r);
static int get_partial_permutation(char *x, int n, int k);
static void set_partial_permutation(char *x, int n, int k, int r);
static int get_permutation(char *x, int n);
static void set_permutation(char *x, int n, int k);
static int get_parity(char *x, int n);

#endif
