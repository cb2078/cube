#ifndef UTIL_H
#define UTIL_H

static int pow2[];
static int pow3[];
static int fact[];
static int powfact4[];
static int pick(int n, int k);
static int choose(int n, int k);

static int get_combination(char *x, int n, int k);
static void set_combination(char *x, int n, int k, int r);
static int get_partial_permutation(char *x, int n, int k);
static void set_partial_permutation(char *x, int n, int k, int r);
static int get_permutation(char *x, int n);
static void set_permutation(char *x, int n, int k);
static int get_parity(char *x, int n);

#endif
