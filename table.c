static table *table_new(long long entries, int bits, char *filename)
{
    assert(bits==1 || bits==2 || bits==4 || bits==8);
    long long size = (entries*bits+7)/8;
    table *t = calloc(1, sizeof(table)+size);
    if (!t)
    {
        fprintf(stderr, "failed to allocate table: %s\n", filename);
        exit(1);
    }
    t->bits = bits;
    t->size = size;
    t->mask = (1<<t->bits)-1;
    t->divisor = (int)sizeof(t->data[0])*8/t->bits;
    if (filename)
    {
        strcpy(t->filename, filename);
        strcat(t->filename, ".bin");
    }
    return t;
}

static void table_destroy(table *t)
{
    free(t);
}

static int table_read(table *t)
{
    FILE *f = fopen(t->filename, "r");
    if (f)
    {
        fread(t->data, t->size, 1, f);
        fclose(f);
        printf("read '%s'\n", t->filename);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int table_write(table *t)
{
    FILE *f = fopen(t->filename, "w");
    if (f)
    {
        fwrite(t->data, t->size, 1, f);
        fclose(f);
        printf("wrote '%s'\n", t->filename);
        return 1;
    }
    else
    {
        printf("couldn't write '%s'\n", t->filename);
        return 0;
    }
}

static void table_set(table *t, long long i, int x)
{
    assert(x<=t->mask);
    int shift = (int)(i&(t->divisor-1))*t->bits;
    t->data[i/t->divisor] = (t->data[i/t->divisor]&~((unsigned)t->mask<<shift)) | (unsigned)x<<shift;
}

static int table_get(table *t, long long i)
{
    int shift = (int)(i&(t->divisor-1))*t->bits;
    return (t->data[i/t->divisor]>>shift)&t->mask;
}
