#include "common.h"

table table_new(int entries, int bits, char *filename)
{
    assert(bits==1 || bits==2 || bits==4);
    table t;
    t.bits = bits;
    t.size = (entries*bits+7)/8;
    t.mask = (1<<t.bits)-1;
    t.filename = filename;
    t.data = malloc(t.size);
    t.divisor = sizeof(t.data[0])*8/t.bits;
    return t;
}

int table_read(table t)
{
    FILE *f = fopen(t.filename, "r");
    if (!f) return 0;
    fread(t.data, t.size, 1, f);
    fclose(f);
    puts("read table");
    return 1;
}

int table_write(table t)
{
    FILE *f = fopen(t.filename, "w");
    if (!f) return 0;
    fwrite(t.data, t.size, 1, f);
    fclose(f);
    puts("write table");
    return 1;
}

void table_set(table t, int i, int x)
{
    assert(x<=t.mask);
    t.data[i/t.divisor] |= x<<(i&t.divisor-1);
}

int table_get(table t, int i)
{
    return (t.data[i/t.divisor]>>(i&t.divisor-1))&t.mask;
}
