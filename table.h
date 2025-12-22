#ifndef TABLE_H
#define TABLE_H

struct table
{
    long long size; // size in bytes
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    unsigned data[];
};

static struct table *table_new(long long, int);

static int table_get(struct table *, long long);
static void table_set(struct table *, long long, int);
#endif
