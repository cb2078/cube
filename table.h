#ifndef TABLE_H
#define TABLE_H

typedef struct
{
    long long size; // size in bytes
    long long count;
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    char filename[256];
    unsigned data[];
} table;

static table *table_new(long long size, int bits, char *filename);

static int table_read(table *t);
static int table_write(table *t);

static int table_get(table *t, long long i);
static void table_set(table *t, long long i, int x);

#endif
