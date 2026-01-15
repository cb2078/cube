#ifndef MAP_H
#define MAP_H

#define MAP_CAPACITY (1ll<<MAP_CAPACITY_LOG2)
#define MAP_CAPACITY_LOG2 25
#define MAP_KEY_BITS 60
#define MAP_KEY_MAX ((1ll<<MAP_KEY_BITS)-1)
#define MAP_VAL_BITS 4
#define MAP_VAL_MAX ((1<<MAP_VAL_BITS)-1)

struct map
{
    long long count;
    struct
    {
        unsigned long long key: MAP_KEY_BITS;
        unsigned long long val: MAP_VAL_BITS;
    } data[];
};

static struct map *map_new(void);
static int map_get(struct map *, long long);
static void map_set(struct map *, long long, int);

#endif
