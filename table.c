#include "common.h"

table table_new(long long entries, int bits, char *filename)
{
    assert(bits==1 || bits==2 || bits==4 || bits==8);
    table t;
    t.bits = bits;
    t.size = (entries*bits+7)/8;
    t.mask = (1<<t.bits)-1;
    t.data = calloc(1, t.size);
    t.divisor = (int)sizeof(t.data[0])*8/t.bits;
    strcpy(t.filename, filename);
    strcat(t.filename, ".bin");
    return t;
}

// reconsider table (the struct) allocation
void table_destroy(table t)
{
    free(t.data);
}

int table_read(table t)
{
    FILE *f = fopen(t.filename, "r");
    if (!f) return 0;
    fread(t.data, t.size, 1, f);
    fclose(f);
    printf("read '%s'\n", t.filename);
    return 1;
}

int table_write(table t)
{
    FILE *f = fopen(t.filename, "w");
    if (!f) return 0;
    fwrite(t.data, t.size, 1, f);
    fclose(f);
    printf("wrote '%s'\n", t.filename);
    return 1;
}

void table_set(table t, long long i, int x)
{
    assert(x<=t.mask);
    int shift = (int)(i&(t.divisor-1))*t.bits;
    t.data[i/t.divisor] = (t.data[i/t.divisor]&~(t.mask<<shift)) | x<<shift;
}

int table_get(table t, long long i)
{
    int shift = (int)(i&(t.divisor-1))*t.bits;
    return (t.data[i/t.divisor]>>shift)&t.mask;
}
