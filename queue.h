#ifndef QUEUE_H
#define QUEUE_H

#include "cube.h"

struct queue
{
    long long start, length, capacity;
    cube_t *entries;
};

static struct queue queue_new(long long);
static cube_t queue_get(struct queue *, long long);
static void queue_push(struct queue *, cube_t);
static cube_t queue_pop(struct queue *);

// TODO find a way to merge this with the searc queue in 'solver.c'

#endif