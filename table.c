static unsigned *table_new(long long entries, int bits)
{
    long long size = TABLE_SIZE(entries, bits);
    unsigned *a = aligned_alloc(TABLE_ALIGN, size);
    memset(a, 0xff, size);
    return a;
}

static int table_get(unsigned *a, int bits, long long i)
{
    ASSERT(bits==2 || bits==4);
    int divisor = sizeof(unsigned)*8/bits;
    int shift = (i&(divisor-1))*bits;
    int mask = (1<<bits)-1;
    return (a[i/divisor] >> shift) & mask;
}

static void table_set(unsigned *a, int bits, long long i, int x)
{
    ASSERT(bits==2 || bits==4);
    int divisor = sizeof(unsigned)*8/bits;
    int shift = (i&(divisor-1))*bits;
    int mask = (1<<bits)-1;
    ASSERT(x >= 0);
    ASSERT(x <= mask);
    a[i/divisor] = (a[i/divisor] & ~((unsigned)mask<<shift)) | (unsigned)x<<shift;
}
