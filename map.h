#pragma once

#define MAP_CAPACITY (1ll<<MAP_CAPACITY_LOG2)
#define MAP_CAPACITY_LOG2 23
#define MAP_BITS 64
#define MAP_KEY_BITS (MAP_BITS-MAP_MOVE_BITS-MAP_VAL_BITS)
#define MAP_KEY_MAX ((1ll<<MAP_KEY_BITS)-1)
#define MAP_MOVE_BITS 5
#define MAP_VAL_BITS 4
#define MAP_VAL_MAX ((1ll<<MAP_VAL_BITS)-1)

struct map
{
    long long count;
    struct
    {
        unsigned long long key: MAP_KEY_BITS;
        unsigned long long move: MAP_MOVE_BITS;
        unsigned long long val: MAP_VAL_BITS;
    } data[];
};

static inline struct map *map_new(void);
static inline int map_get(struct map *, long long);
static inline void map_set(struct map *, long long, int, int);
