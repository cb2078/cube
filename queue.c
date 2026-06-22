static struct queue queue_new(long long capacity)
{
    struct queue q = {0};
    q.capacity = capacity;
    q.entries = aligned_alloc(sizeof(cube_t), capacity*sizeof(cube_t));
    return q;
}

static cube_t queue_get(struct queue *q, long long i)
{
    ASSERT(i < q->length);
    return q->entries[(q->start+i)%q->capacity];
}

static void queue_push(struct queue *q, cube_t x)
{
    for (int i=0; i<q->length; i++)
        if (cube_eq(x, queue_get(q, i)))
            return;
    ASSERT(++q->length < q->capacity);
    q->entries[(q->start+q->length-1)%q->capacity] = x;
}

static cube_t queue_pop(struct queue *q)
{
    ASSERT(q->length--);
    long long i = q->start++;
    q->start %= q->capacity;
    return q->entries[i];
}