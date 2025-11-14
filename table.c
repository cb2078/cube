static struct table *table_new(long long entries, int bits, char *name)
{
    assert(bits==1 || bits==2 || bits==4 || bits==8);
    long long size = (entries*bits+7)/8;
    struct table *t = calloc(1, sizeof(struct table)+size);
    if (!t)
    {
        fprintf(stderr, "failed to allocate table: %s\n", name);
        exit(1);
    }
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

static int table_read(struct table *t)
{
    char buf[256];
    sprintf(buf, "%s.prune.bin", t->name);
    FILE *f = fopen(buf, "rb");
    if (f)
    {
        fread(t->data, t->size, 1, f);
        fclose(f);
        fprintf(stderr, "read '%s'\n", buf);
        return 1;
    }
    return 0;
}

static int table_write(struct table *t)
{
    char buf[256];
    sprintf(buf, "%s.prune.bin", t->name);
    FILE *f = fopen(buf, "wb");
    if (f)
    {
        fwrite(t->data, t->size, 1, f);
        fclose(f);
        fprintf(stderr, "wrote '%s'\n", buf);
        return 1;
    }
    else
    {
        fprintf(stderr, "couldn't write '%s'\n", buf);
        return 0;
    }
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
