static int job_thread(void *_)
{
    (void)_;
    for (;;)
    {
        sem_wait(&pool.sem_worker);
        int i = atomic_fetch_add(&pool.start, 1);
        pool.entries[i].f(pool.entries[i].x);
        sem_post(&pool.sem_owner);
    }
    return 0;
}

static void job_dispatch(void (*f)(void *), void *args, long long size)
{
    pool.start=0;
    for (int i=0; i<THREADS; i++)
    {
        pool.entries[i].f = f;
        pool.entries[i].x = args ? args+i*size : (void *)(long long)i;
        sem_post(&pool.sem_worker);
    }
    for (int i=0; i<THREADS; i++)
        sem_wait(&pool.sem_owner);
}