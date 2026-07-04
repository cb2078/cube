#pragma once

#include "cube.h"
#include "solver.h"

struct queue
{
    long long start, length, capacity;
    struct search_node *entries;
};

static inline struct queue queue_new(long long);
static inline struct search_node queue_get(struct queue *, long long);
static inline void queue_push(struct queue *, cube_t, int, int);
static inline struct search_node queue_pop(struct queue *);

// TODO find a way to merge this with the searc queue in 'solver.c'
// NOTE this is not faster when using a linear search when checking for duplicates positions when used in solver.c
