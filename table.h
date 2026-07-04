#ifndef TABLE_H
#define TABLE_H

#define TABLE_ALIGN 16

#define TABLE_SIZE(entries, bits) ((((entries)*(bits)+7)/8)+15)/16*16
#define TABLE_SET_MIN(t, bits, i, x) (void)(table_get((t), (bits), (i)) > (x) ? table_set((t), (bits), (i), (x)) : 0)

static inline unsigned *table_new(long long, int);
static inline int table_get(unsigned *, int, long long);
static inline void table_set(unsigned *, int, long long, int);
#endif
