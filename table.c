static struct table *table_new(long long entries, int bits)
{
    ASSERT(bits==1 || bits==2 || bits==4 || bits==8);
    long long size = (entries*bits+7)/8;
    struct table *t = malloc(sizeof(struct table)+size);
    t->bits = bits;
    t->size = size;
    t->mask = (1<<t->bits)-1;
    t->divisor = (int)sizeof(t->data[0])*8/t->bits;
    memset(t->data, 0xff, size);
    return t;
}

static int table_get(struct table *t, long long i)
{
    int shift = (int)(i&(t->divisor-1))*t->bits;
    return (t->data[i/t->divisor]>>shift)&t->mask;
}

static void table_set(struct table *t, long long i, int x)
{
    ASSERT(x >= 0);
    ASSERT(x <= t->mask);
    int shift = (int)(i&(t->divisor-1))*t->bits;
    t->data[i/t->divisor] = (t->data[i/t->divisor]&~((unsigned)t->mask<<shift))
        | (unsigned)x<<shift;
}
