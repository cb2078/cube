static struct table *table_new(long long entries, int bits, char *name)
{
    assert(bits==1 || bits==2 || bits==4 || bits==8);
    long long size = (entries*bits+7)/8;
    struct table *t = calloc(1, sizeof(struct table)+size);
    if (!t)
        ERROR("failed to allocate memory for '%s' table\n", name);
    t->bits = bits;
    t->size = size;
    t->mask = (1<<t->bits)-1;
    t->divisor = (int)sizeof(t->data[0])*8/t->bits;
    t->name = name;
    return t;
}

static void table_destroy(struct table *t)
{
    free(t);
}

static void table_set(struct table *t, long long i, int x)
{
    assert(x<=t->mask);
    int shift = (int)(i&(t->divisor-1))*t->bits;
    t->data[i/t->divisor] = (t->data[i/t->divisor]&~((unsigned)t->mask<<shift)) | (unsigned)x<<shift;
    t->count++;
}

static int table_get(struct table *t, long long i)
{
    int shift = (int)(i&(t->divisor-1))*t->bits;
    return (t->data[i/t->divisor]>>shift)&t->mask;
}
