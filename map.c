static inline struct map *map_new(void)
{
    long long size = MAP_CAPACITY*8;
    struct map *m = malloc(sizeof(struct map)+size);
    m->count = 0;
    memset(m->data, 0xff, size);
    return m;
}

// https://en.wikipedia.org/wiki/Hash_function#Fibonacci_hashing
static inline int hash(unsigned long long k)
{
    unsigned long long a = 11400714819323198485ull;
    int w = 64;
    int m = MAP_CAPACITY_LOG2;
    k ^= k >> (w - m);
    return (a * k) >> (w - m);
}

static inline int map_index(struct map *m, long long k)
{
    ASSERT(k < MAP_KEY_MAX);
    int i = hash(k);
    while (m->data[i].val != MAP_VAL_MAX && m->data[i].key != k)
        i = i<MAP_CAPACITY-1 ? i+1 : 0;
    return i;
}

static inline int map_get(struct map *m, long long k)
{
    int i = map_index(m, k);
    return m->data[i].val;
}

static inline void map_set(struct map *m, long long k, int v)
{
    ASSERT(v != MAP_VAL_MAX);
    int i = map_index(m, k);
    m->data[i].key = k;
    m->data[i].val = v;
    m->count++;
}
