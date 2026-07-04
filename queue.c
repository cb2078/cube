static inline struct queue queue_new(long long capacity)
{
    struct queue q = {0};
    q.capacity = capacity;
    q.entries = aligned_alloc(sizeof(struct search_node), capacity*sizeof(struct search_node));
    return q;
}

static inline struct search_node queue_get(struct queue *q, long long i)
{
    ASSERT(i < q->start+q->length);
    return q->entries[q->start+i];
}

static inline void queue_push(struct queue *q, cube_t x, int move, int depth)
{
    for (long long i=0; i<q->start+q->length; i++)
        if (cube_eq(x, q->entries[i].cube))
            return;
    ASSERT(q->start+(++q->length) < q->capacity);
    q->entries[q->start+q->length-1] = (struct search_node){x, move, depth};
}

static inline struct search_node queue_pop(struct queue *q)
{
    ASSERT(q->length--);
    long long i = q->start++;
    q->start %= q->capacity;
    return q->entries[i];
}