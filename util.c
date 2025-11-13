static int get_parity(char *x, int n)
{
    int r = 0;
    for (int i=0; i<n; ++i)
        for (int j=i+1; j<n; ++j)
            r ^= x[i]<x[j];
    return r;
}

static void fill_remaining(char *x, int n, int k, unsigned b)
{
    for (int i=0; i<n; ++i)
        if (~b>>(n-1-i)&1)
            x[i] = k++;
}

static int get_combination(char *x, int n, int k)
{
    int r=0;
    for (int i=0, j=0; i<n; ++i)
        if (x[i] < k)
            r += choose[i][++j];
    return r;
}

static void set_combination(char *x, int n, int k, int r)
{
    unsigned b = 0;
    for (int i=0, j=k; i<n; ++i)
    {
        if (r-choose[n-1-i][j] >= 0)
        {
            x[n-1-i] = j-1;
            r -= choose[n-1-i][j--];
            b |= 1<<i;
        }
    }
    fill_remaining(x, n, k, b);
}

static int get_partial_permutation(char *x, int n, int k)
{
    unsigned b = 0;
    int r = 0;
    for (int i=0; i<n; ++i)
    {
        b |= 1<<(n-1-x[i]);
        unsigned s = __builtin_popcount(b>>(n-x[i]));
        int c = i-s;
        if (k > x[i])
            r += c*pick[n-1-x[i]][k-1-x[i]];
    }
    return r;
}

static void set_partial_permutation(char *x, int n, int k, int r)
{
    unsigned b = 0;
    for (int i=0; i<k; ++i)
    {
        int c = r/pick[n-1-i][k-1-i];
        int s=0, j=0;
        while (s+=b>>(n-1-j)&1, j<c+s)
            ++j;
        x[j] = i;
        r %= pick[n-1-i][k-1-i];
        b |= 1<<(n-1-j);
    }
    fill_remaining(x, n, k, b);
}

static int get_permutation(char *x, int n)
{
    return get_partial_permutation(x, n, n);
}

static void set_permutation(char *x, int n, int r)
{
    set_partial_permutation(x, n, n, r);
}
