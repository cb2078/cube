#ifndef TABLE_H
#define TABLE_H

#define TABLE_ALIGN 16

#define TABLE_SIZE(entries, bits) (((entries)*(bits)+7)/8)

static unsigned *table_new(long long, int);
static int table_get(unsigned *, int, long long);
static void table_set(unsigned *, int, long long, int);
#endif
