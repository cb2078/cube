#ifndef TABLE_H
#define TABLE_H

struct table
{
    long long size; // size in bytes
    _Atomic long long count;
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    unsigned data[];
};

static struct table *table_new(long long size, int bits);

static int table_get(struct table *t, long long i);
static void table_set(struct table *t, long long i, int x);

static int table_get_atomic(struct table *t, long long i);
static void table_set_atomic(struct table *t, long long i, int x);

#endif
