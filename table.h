#ifndef TABLE_H
#define TABLE_H

struct table
{
    long long size; // size in bytes
    long long count;
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    char *name;
    unsigned data[];
};

static struct table *table_new(long long size, int bits, char *name);
static void table_destroy(struct table *t);
static int table_get(struct table *t, long long i);
static void table_set(struct table *t, long long i, int x);

#endif
