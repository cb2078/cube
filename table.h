typedef struct table table;
struct table
{
    long long size; // size in bytes
    int bits; // bits per entry
    int mask; // (1<<bits)-1
    int divisor; // sizeof(data[0])/bits
    char filename[256];
    unsigned data[];
};

table *table_new(long long size, int bits, char *filename);
int table_read(table *t);
int table_write(table *t);
void table_set(table *t, long long i, int x);
int table_get(table *t, long long i);
