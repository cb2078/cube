#ifndef JOB_H
#define JOB_H

static struct
{
    sem_t sem_worker, sem_owner;
    int start;
    struct
    {
        void (*f)(void *);
        void *x;
    } *entries;
} pool;

static int job_thread(void *);
static void job_dispatch(void (*f)(void *), void *, long long);

#endif