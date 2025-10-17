#include "util.h"

#define SWAP(x, y) do { typeof(x) z=x; x=y; y=z; } while (0)

int pow2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
int pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};
int fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600};
int powfact4[] = {1, 24, 576, 13824, 331776};

int get_parity(char *x, int n)
{
    int r = 0;
    for (int i=0; i<n; ++i)
        for (int j=i+1; j<n; ++j)
            r ^= (x[i]&0x0f)<(x[j]&0x0f);
    return r;
}

int pick(int n, int k)
{
    return n>=k ? fact[n]/fact[n-k] : 0;
}

int choose(int n, int k)
{
    return n>=k ? fact[n]/fact[n-k]/fact[k] : 0;
}

static void fill_remaining(char *x, int n, int k, unsigned b)
{
    for (int i=0; i<n; ++i)
        if (~b>>(n-1-i)&1)
            x[i] = k++;
}

int get_combination(char *x, int n, int k)
{
    int r=0;
    for (int i=0, j=0; i<n; ++i)
        if (x[i] < k)
            r += choose(i, ++j);
    return r;
}

void set_combination(char *x, int n, int k, int r)
{
    unsigned b = 0;
    for (int i=0, j=k; i<n; ++i)
    {
        if (r-choose(n-1-i, j) >= 0)
        {
            x[n-1-i] = j-1;
            r -= choose(n-1-i, j--);
            b |= 1<<i;
        }
    }
    fill_remaining(x, n, k, b);
}

int get_partial_permutation(char *x, int n, int k)
{
    unsigned b = 0;
    int r = 0;
    for (int i=0; i<n; ++i)
    {
        b |= 1<<(n-1-x[i]);
        unsigned s = __builtin_popcount(b>>(n-x[i]));
        int c = i-s;
        r += c*pick(n-1-x[i], k-1-x[i]);
    }
    return r;
}

void set_partial_permutation(char *x, int n, int k, int r)
{
    unsigned b = 0;
    for (int i=0; i<k; ++i)
    {
        int c = r/pick(n-1-i, k-1-i);
        int s=0, j=0;
        while (s+=b>>(n-1-j)&1, j<c+s)
            ++j;
        x[j] = i;
        r %= pick(n-1-i, k-1-i);
        b |= 1<<(n-1-j);
    }
    fill_remaining(x, n, k, b);
}

int get_permutation(char *x, int n)
{
    return get_partial_permutation(x, n, n);
}

void set_permutation(char *x, int n, int r)
{
    set_partial_permutation(x, n, n, r);
}
